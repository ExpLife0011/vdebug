
#include <Windows.h>
#include <Psapi.h>
#include <winternl.h>
#include <Shlwapi.h>
#include <AccCtrl.h>
#include <AclAPI.h>
#include <WtsApi32.h>
#include <UserEnv.h>
#include "ComStatic.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "WtsApi32.lib")

using namespace std;

//Dos文件路径转为Nt路径
mstring __stdcall DosPathToNtPath(LPCSTR szSrc)
{
    DWORD dwDrivers = GetLogicalDrives();
    int iIdex = 0;
    char szNT[] = "X:";
    char szDos[MAX_PATH] = {0x00};
    mstring strHeader;
    mstring strNtPath;
    mstring strDos(szSrc);
    for (iIdex = 0 ; iIdex < 26 ; iIdex++)
    {
        if ((1 << iIdex) & dwDrivers)
        {
            szNT[0] = 'A' + iIdex;
            if (QueryDosDeviceA(szNT, szDos, MAX_PATH))
            {
                if (0 == strDos.comparei(szDos))
                {
                    strNtPath += szNT;
                    strNtPath += (strDos.c_str() + lstrlenA(szDos));
                    return strNtPath;
                }
            }
        }
    }
    return "";
}

mstring __stdcall GetProcPathByPid(IN DWORD dwPid)
{
    if (4 == dwPid || 0 == dwPid)
    {
        return "";
    }

    mstring strPath;
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

    char image[MAX_PATH] = {0x00};
    if (process)
    {
        GetProcessImageFileNameA(process, image, MAX_PATH);
        if (image[4] != 0x00)
        {
            strPath = DosPathToNtPath(image);
        }
    }
    else
    {
        return "";
    }

    if (process && INVALID_HANDLE_VALUE != process)
    {
        CloseHandle(process);
    }
    return strPath;
}

mstring __stdcall GetFilePathFromHandle(HANDLE hFile)
{
    DWORD dwFileSizeLow = GetFileSize(hFile, NULL); 
    HANDLE hFileMap = NULL;
    void* pMem = NULL;
    mstring strPath;

    do 
    {
        if (!dwFileSizeLow)
        {
            break;
        }

        hFileMap = CreateFileMapping(hFile, 
            NULL, 
            PAGE_READONLY,
            0, 
            1,
            NULL
            );
        if (!hFileMap)
        {
            break;
        }

        pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
        if (!pMem)
        {
            break;
        }

        CHAR szFileName[MAX_PATH] = {0};
        GetMappedFileNameA(
            GetCurrentProcess(), 
            pMem, 
            szFileName,
            MAX_PATH
            );
        if (!szFileName[0])
        {
            break;
        }

        strPath = DosPathToNtPath(szFileName);
    } while (FALSE);

    if (hFileMap)
    {
        CloseHandle(hFileMap);
    }

    if (pMem)
    {
        UnmapViewOfFile(pMem);
    }
    return strPath;
}

mstring __stdcall GetStdErrorStr
(DWORD dwErr)
{
    LPVOID lpMsgBuf = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |  
        FORMAT_MESSAGE_FROM_SYSTEM |  
        FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), //Default language  
        (LPSTR)&lpMsgBuf,  
        0,  
        NULL  
        ); 
    mstring strMsg((LPCSTR)lpMsgBuf);
    if (lpMsgBuf)
    {
        LocalFlags(lpMsgBuf);
    }
    return strMsg;
}

// 注意，此函数的第二个参数很畸形，如果在本函数内声明 PACL* 并且释放的话，则安全描述符失效
// 为了应对此问题，需要调用方提供 PACL* 并在调用完此函数后自行释放内存
static BOOL WINAPI _SecGenerateLowSD(SECURITY_DESCRIPTOR* pSecDesc, PACL* pDacl)
{
    PSID pSidWorld = NULL;
    EXPLICIT_ACCESS ea;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
    BOOL bRet = FALSE;

    do
    {
        if (!pSecDesc || !pDacl)
        {
            break;
        }

        if (AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSidWorld) == 0)
        {
            break;
        }

        ea.grfAccessMode = GRANT_ACCESS;
        ea.grfAccessPermissions = FILE_ALL_ACCESS ;
        ea.grfInheritance = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
        ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ea.Trustee.pMultipleTrustee = NULL;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName = (LPTSTR)pSidWorld;

        if (SetEntriesInAcl(1, &ea, NULL, pDacl) != ERROR_SUCCESS)
        {
            break;
        }

        if (InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION) == 0)
        {
            break;
        }

        if (SetSecurityDescriptorDacl(pSecDesc, TRUE, *pDacl, FALSE) == 0)
        {
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    if (NULL != pSidWorld)
    {
        FreeSid(pSidWorld);
        pSidWorld = NULL;
    }
    return bRet;
}

HANDLE WINAPI CreateLowsdEvent(BOOL bReset, BOOL bInitStat, LPCSTR szName)
{
    SECURITY_DESCRIPTOR secDesc;
    PACL pDacl = NULL;
    SECURITY_ATTRIBUTES secAttr;
    secAttr.nLength = sizeof(secAttr);
    secAttr.lpSecurityDescriptor = &secDesc;
    secAttr.bInheritHandle = FALSE;
    if (!_SecGenerateLowSD(&secDesc, &pDacl))
    {
        if (pDacl)
        {
            LocalFree(pDacl);
        }
        return FALSE;
    }

    HANDLE hEvent = CreateEventA(&secAttr, bReset, bInitStat, szName);
    if (pDacl)
    {
        LocalFree(pDacl);
    }
    return hEvent;
}

static DWORD _GetCurSessionId()
{
    WTS_SESSION_INFOW *pSessions = NULL;
    DWORD dwSessionCount = 0;
    DWORD dwActiveSession = -1;
    WTSEnumerateSessionsW(
        WTS_CURRENT_SERVER_HANDLE,
        0,
        1,
        &pSessions,
        &dwSessionCount
        );
    DWORD dwIdex = 0;
    for (dwIdex = 0 ; dwIdex < dwSessionCount ; dwIdex++)
    {
        if (WTSActive == pSessions[dwIdex].State)
        {
            dwActiveSession = pSessions[dwIdex].SessionId;
            break;
        }
    }
    if (pSessions)
    {
        WTSFreeMemory(pSessions);
    }

    return dwActiveSession;
}

static HANDLE _GetProcessToken(DWORD dwPid)
{
    BOOL bRet = FALSE;
    HANDLE hToken = NULL;
    HANDLE hDup = NULL;
    HANDLE hProcess = NULL;
    do
    {
        if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid)))
        {
            break;
        }

        if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
        {
            break;
        }

        if (!DuplicateTokenEx(
            hToken,
            MAXIMUM_ALLOWED,
            NULL,
            SecurityIdentification,
            TokenPrimary,
            &hDup
            ))
        {
            break;
        }
    } while(FALSE);

    if (hToken)
    {
        CloseHandle(hToken);
    }

    if (hProcess)
    {
        CloseHandle(hProcess);
    }
    return hDup;
}

BOOL WINAPI RunInSession(LPCSTR szImage, LPCSTR szCmd, DWORD dwSessionId, DWORD dwShell)
{
    if (!szImage || !*szImage)
    {
        return FALSE;
    }

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(szImage))
    {
        return FALSE;
    }

    BOOL bStat = FALSE;
    HANDLE hThis = NULL;
    HANDLE hDup = NULL;
    LPVOID pEnv = NULL;
    HANDLE hShellToken = NULL;
    DWORD  dwFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;
    do
    {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hThis))
        {
            break;
        }

        if (!DuplicateTokenEx(hThis, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hDup))
        {
            break;
        }

        if (!dwSessionId)
        {
            dwSessionId = _GetCurSessionId();
        }

        if (-1 == dwSessionId)
        {
            break;
        }
        if (!SetTokenInformation(hDup, TokenSessionId, &dwSessionId, sizeof(DWORD)))
        {
            break;
        }

        if (dwShell)
        {
            hShellToken = _GetProcessToken(dwShell);
            CreateEnvironmentBlock(&pEnv, hShellToken, FALSE);
        }
        else
        {
            CreateEnvironmentBlock(&pEnv, hDup, FALSE);
        }

        CHAR szParam[1024] = {0};
        if (szCmd && szCmd[0])
        {
            wnsprintfA(szParam, 1024, "\"%hs\" \"%hs\"", szImage, szCmd);
        }
        else
        {
            wnsprintfA(szParam, 1024, "\"%hs\"", szImage);
        }
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(STARTUPINFO);
        si.lpDesktop = "WinSta0\\Default";
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = TRUE;
        bStat = CreateProcessAsUserA(
            hDup,
            NULL,
            szParam,
            NULL,
            NULL,
            FALSE,
            dwFlag,
            pEnv,
            NULL,
            &si,
            &pi
            );
        if (pi.hProcess)
        {
            CloseHandle(pi.hProcess);
        }

        if (pi.hThread)
        {
            CloseHandle(pi.hThread);
        }

        if (!bStat)
        {
        }
    } while (FALSE);

    if (pEnv)
    {
        DestroyEnvironmentBlock(pEnv);
        pEnv = NULL;
    }

    if (hThis && INVALID_HANDLE_VALUE != hThis)
    {
        CloseHandle(hThis);
        hThis = NULL;
    }

    if (hShellToken && INVALID_HANDLE_VALUE != hShellToken)
    {
        CloseHandle(hShellToken);
        hShellToken = NULL;
    }

    if (hDup && INVALID_HANDLE_VALUE != hDup)
    {
        CloseHandle(hDup);
        hDup = NULL;
    }
    return TRUE;
}