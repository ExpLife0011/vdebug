#include <WinSock2.h>
#include <Windows.h>
#include "view.h"
#include "SyntaxHlpr/SyntaxParser.h"
#include <runner/runner.h>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>
#include <Shlwapi.h>
#include "DbgCtrlService.h"
#include "OpenView.h"

#pragma comment(lib, "Ws2_32.lib")

HINSTANCE g_hInstance = NULL;

static void _TestProc() {
    {
        MemoryAlloc<int> allocer;
        int *ptr = allocer.GetMemory(1024);
    }

    SqliteOperator db;
    char path[256];
    GetModuleFileNameA(NULL, path, 256);
    PathAppendA(path, "..\\test1.db");
    bool f = db.Open(path);

    mstring createSql = "CREATE TABLE testtable1("  \
    "id INTEGER PRIMARY KEY,"                       \
    "name           TEXT    NOT NULL,"              \
    "age            INT     NOT NULL,"              \
    "address        CHAR(50),"                      \
    "msg         TEXT)";
    db.Exec(createSql.c_str());

    mstring err = db.GetError();

    mstring insertSql = "insert into testtable1(name, age, address, msg)values('test1', 11, 'address1', 'msg1')";
    db.Insert(insertSql.c_str());
    err = db.GetError();

    mstring selectSql = "select * from testtable1";
    SqliteResult result = db.Select(selectSql.c_str());
    err = db.GetError();
    int dd = 123;

    int ss = 0;
    for (SqliteIterator it = result.begin() ; it != result.end() ; it = it.GetNext())
    {
        int ff = 456;
        ss++;
    }
    int ee = 0;
    //InitSymbolHlpr(L"SRV*F:\\mysymbol*http://msdl.microsoft.com/download/symbols/");
    //InitSymbolHlpr(L"F:\\mysymbol");

    //WCHAR wsz[MAX_PATH] = {0};
    //GetModuleFileNameW(NULL, wsz, MAX_PATH);
    //PathAppendW(wsz, L"..\\112233.dmp");
    //GetMiniDumpHlpr()->LodeDump(wsz);

    //list<DumpModuleInfo> modules;
    //GetMiniDumpHlpr()->GetModuleSet(modules);

    //list<DumpThreadInfo> threads;
    //GetMiniDumpHlpr()->GetThreadSet(threads);

    //DumpSystemInfo system;
    //GetMiniDumpHlpr()->GetSystemInfo(system);

    //list<DumpMemoryInfo> memorys;
    //GetMiniDumpHlpr()->GetMemoryInfo(memorys);

    //GetMiniDumpHlpr()->GetCallStack();

    //MessageBoxW(0, 0, 0, 0);
}

static BOOL _StartViewProc() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    if (!LoadLibraryW(L"SyntaxView.dll"))
    {
        MessageBoxW(NULL, L"SyntaxView.dll not Find", L"Error", 0);
        return FALSE;
    }

    if (!LoadLibraryW(L"ComLib32.dll"))
    {
        MessageBoxW(NULL, L"ComLib32.dll not Find", L"Error", 0);
        return FALSE;
    }

    SyntaxParser::GetInstance()->InitParser();
    CoInitialize(NULL);
    ShowMainView();
    CoUninitialize();
    WSACleanup();
    return TRUE;
}

static BOOL _StartService() {
    CHAR szRunner[MAX_PATH] = {0};
    CHAR szServ[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, szRunner, MAX_PATH);
    PathAppendA(szRunner, "../runner.exe");

    GetWindowsDirectoryA(szServ, MAX_PATH);
    PathAppendA(szServ, "DbgService.exe");

    BOOL bServ = TRUE;
    BOOL bStat = FALSE;
    do 
    {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(szServ))
        {
            CopyFileA(szRunner, szServ, FALSE);
        }
        else
        {
            if (!IsSameFileA(szRunner, szServ))
            {
                mstring strTemp(szServ);
                mstring strName;
                strName.format("..\\DbgService%08x.tmp", GetTickCount());
                strTemp.path_append(strName.c_str());

                MoveFileExA(szServ, strTemp.c_str(), MOVEFILE_REPLACE_EXISTING);
                MoveFileExA(strTemp.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                CopyFileA(szRunner, szServ, FALSE);
                ServStopA(SFV_SERVICE_NAME);
                ServStartA(SFV_SERVICE_NAME);
            }
        }

        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(szServ))
        {
            break;
        }

        HANDLE hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
        HandleAutoClose abc(hNotify);

        if (hNotify)
        {
            dp(L"��������Ѿ�����");
            bStat = TRUE;
            break;
        }

        PathQuoteSpacesA(szServ);
        mstring cmd = FormatA("%hs -service", szServ);

        bServ = (InstallLocalServiceA(cmd.c_str(), SFV_SERVICE_NAME, SFV_SERVICE_DISPLAY_NAME, SFV_SERVICE_DESCRIPTION) && StartLocalServiceA(SFV_SERVICE_NAME));
        if (bServ)
        {
            hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
            DWORD dwTimeCount = GetTickCount();
            while (GetTickCount() - dwTimeCount < 5000)
            {
                if (hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME))
                {
                    bStat = TRUE;
                    break;
                }
                Sleep(10);
            }
        }
        else
        {
            dp(L"����SfvServ����ʧ��:%d", GetLastError());
        }
    } while (FALSE);
    return bStat;
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    g_hInstance = m;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    LoadLibraryW(L"ComLib32.dll");
    LoadLibraryW(L"mq32.dll");
    LoadLibraryW(L"DbgCtrl32.dll");

    DbProxy::GetInstance()->InitDbEnv();

    _StartService();
    DbgCtrlService::GetInstance()->InitCtrlService();
    _StartViewProc();
    WSACleanup();
    return 0;
}