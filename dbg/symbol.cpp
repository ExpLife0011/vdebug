#include <Windows.h>
#include <DbgHelp.h>
#include "symbol.h"
#include <ComLib/ComLib.h>
//#include "view/MainView.h"
#include <ComLib/ComLib.h>

CSymbolHlpr::CSymbolHlpr(const ustring &wstrSymbolPath)
{
    m_hNotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hInitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hDbgProc = NULL;
    m_wstrSymbolPath = wstrSymbolPath;
    CloseHandle(CreateThread(NULL, 0, WorkThread, this, 0, 0));
    WaitForSingleObject(m_hInitEvent, INFINITE);
}

bool CSymbolHlpr::SetSymbolPath(const ustring &wstrSymbolPath)
{
    CSymbolTaskHeader task;
    task.m_dwSize = sizeof(CSymbolTaskHeader);
    task.m_eTaskType = em_task_setpath;
    task.m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
    task.m_pParam = NULL;
    SendTask(&task);
    return true;
}

CSymbolHlpr:: ~CSymbolHlpr()
{}

bool CSymbolHlpr::SendTask(CSymbolTaskHeader *pTask, BOOL bAtOnce)
{
    {
        CScopedLocker lock(this);
        pTask->m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (bAtOnce)
        {
            m_vTaskQueue.push_front(pTask);
        }
        else
        {
            m_vTaskQueue.push_back(pTask);
        }
        SetEvent(m_hNotifyEvent);
    }

    WaitForSingleObject(pTask->m_hNotify, INFINITE);
    CloseHandle(pTask->m_hNotify);
    return true;
}

bool CSymbolHlpr::PostTask(CSymbolTaskHeader *pTask, BOOL bAtOnce)
{
    {
        CScopedLocker(this);
        if (bAtOnce)
        {
            m_vTaskQueue.push_front(pTask);
        }
        else
        {
            m_vTaskQueue.push_back(pTask);
        }
    }
    SetEvent(m_hNotifyEvent);
    return true;
}

bool CSymbolHlpr::IsSymbolLoaded(const ustring &wstrDll, DWORD64 dwBaseOfModule)
{
    CScopedLocker lock(this);
    for (list<SymbolLoadInfo>::iterator it = m_vSymbolInfo.begin() ; it != m_vSymbolInfo.end() ; it++)
    {
        if (it->m_dwBaseOfModule == dwBaseOfModule && (0 == it->m_wstrMuduleName.comparei(wstrDll)))
        {
            return true;
        }
    }
    return false;
}

bool CSymbolHlpr::UnLoadModuleByName(const ustring &wstrDllName)
{
    CScopedLocker lock(this);
    for (list<SymbolLoadInfo>::iterator it = m_vSymbolInfo.begin() ; it != m_vSymbolInfo.end() ; it++)
    {
        if (0 == it->m_wstrMuduleName.comparei(wstrDllName))
        {
            SymUnloadModule64(NULL, it->m_dwBaseOfModule);
            m_vSymbolInfo.erase(it);
            return true;
        }
    }
    return false;
}

bool CSymbolHlpr::UnLoadModuleByAddr(DWORD64 dwAddr)
{
    CScopedLocker lock(this);
    for (list<SymbolLoadInfo>::iterator it = m_vSymbolInfo.begin() ; it != m_vSymbolInfo.end() ; it++)
    {
        if (it->m_dwBaseOfModule == dwAddr)
        {
            SymUnloadModule64(GetSymbolHlpr()->m_hDbgProc, dwAddr);
            m_vSymbolInfo.erase(it);
            return true;
        }
    }
    return false;
}

bool CSymbolHlpr::UnloadAllModules()
{
    CScopedLocker lock(this);
    for (list<SymbolLoadInfo>::iterator it = m_vSymbolInfo.begin() ; it != m_vSymbolInfo.end() ; it++)
    {
        SymUnloadModule64(GetSymbolHlpr()->m_hDbgProc, it->m_dwBaseOfModule);
    }
    m_vSymbolInfo.clear();
    return true;
}

bool CSymbolHlpr::LoadModule(HANDLE hFile, const ustring &wstrDllPath, DWORD64 dwBaseOfModule)
{
    CScopedLocker lock(this);
    DWORD64 dwBaseAddr = SymLoadModuleExW(
        m_hDbgProc,
        hFile,
        wstrDllPath.c_str(),
        0,
        dwBaseOfModule,
        0,
        0,
        0
        );
    if (dwBaseAddr)
    {
        SymbolLoadInfo info;
        info.m_dwBaseOfModule = dwBaseAddr;
        info.m_wstrModulePath = wstrDllPath;
        info.m_wstrMuduleName = PathFindFileNameW(wstrDllPath.c_str());
        m_vSymbolInfo.push_back(info);
        return true;
    }
    return false;
}

bool CSymbolHlpr::InitEngine(CTaskSymbolInit *pInitInfo)
{
    SymSetOptions(SymGetOptions() | SYMOPT_CASE_INSENSITIVE | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS);
    SymInitializeW(pInitInfo->m_hDstProc, NULL, FALSE);
    GetSymbolHlpr()->m_hDbgProc = pInitInfo->m_hDstProc;
    return true;
}

bool CSymbolHlpr::LoadSymbol(CTaskLoadSymbol *pModuleInfo)
{
    ustring wstrDll = GetFilePathFromHandle(pModuleInfo->m_hImgaeFile);
    ustring wstrName = PathFindFileNameW(wstrDll.c_str());
    SetLastError(0);
    DWORD64 dwBaseOfModule = (DWORD64)pModuleInfo->m_dwBaseOfModule;
    DWORD64 dwBaseAddr = dwBaseOfModule;
    if (!GetSymbolHlpr()->IsSymbolLoaded(wstrName, dwBaseOfModule))
    {
        GetSymbolHlpr()->UnLoadModuleByName(wstrName);
        if (!GetSymbolHlpr()->LoadModule(pModuleInfo->m_hImgaeFile, wstrDll, dwBaseOfModule))
        {
            return false;
        }
    }

    ustring wstr1(wstrDll);
    wstr1.makelower();
    if (ustring::npos != wstr1.find(L"kernelbase.dll"))
    {
        ustring wstrError = GetStdErrorStr();

        char szBuffer[4096] = {0};
        SYMBOL_INFOW *pTest = (SYMBOL_INFOW *)szBuffer;
        pTest->SizeOfStruct  = sizeof(SYMBOL_INFOW);
        pTest->MaxNameLen = 1024;
        pTest->ModBase = dwBaseOfModule;
        SymFromNameW(NULL, L"kernelbase!createfileW", pTest);
        int dd = 0;
    }

    IMAGEHLP_MODULEW64 info = {0};
    info.SizeOfStruct = sizeof(info);
    SymGetModuleInfoW64(GetSymbolHlpr()->m_hDbgProc, (DWORD64)dwBaseAddr, &info);

    DbgModuleInfo *pModule = &(pModuleInfo->m_ModuleInfo);
    pModule->m_wstrDllPath = wstrDll;
    pModule->m_wstrDllName = PathFindFileNameW(wstrDll.c_str());
    pModule->m_dwBaseOfImage = dwBaseAddr;
    pModule->m_dwModuleSize = info.ImageSize;
    pModule->m_dwEndAddr = (pModule->m_dwBaseOfImage + pModule->m_dwModuleSize);
    pModule->m_hModule = (HMODULE)dwBaseAddr;
    return true;
}

bool CSymbolHlpr::GetSymbolFromAddr(CTaskSymbolFromAddr *pSymbolInfo)
{
    DWORD64 dwOffset = 0;
    ULONG64 buffer[(sizeof(SYMBOL_INFO) +
        MAX_SYM_NAME*sizeof(WCHAR) +
        sizeof(ULONG64) - 1) /
        sizeof(ULONG64)] = {0};
    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;

    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    DWORD64 dwBaseOfModule = (DWORD64)pSymbolInfo->m_ModuleInfo.m_dwBaseOfImage;
    if (!GetSymbolHlpr()->IsSymbolLoaded(pSymbolInfo->m_ModuleInfo.m_wstrDllName, dwBaseOfModule))
    {
        return false;
    }

    if (!SymFromAddrW(GetSymbolHlpr()->m_hDbgProc, pSymbolInfo->m_dwAddr, &dwOffset, pSymbol))
    {
        ustring wstrErr = GetStdErrorStr();
        pSymbolInfo->m_wstrSymbol = FormatW(L"0x%x", pSymbolInfo->m_dwAddr - dwBaseOfModule);
        return false;
    }

    if (dwOffset)
    {
        pSymbolInfo->m_wstrSymbol = FormatW(L"%ls+0x%x", pSymbol->Name, dwOffset);
    }
    else
    {
        pSymbolInfo->m_wstrSymbol = pSymbol->Name;
    }
    return true;
}

bool CSymbolHlpr::GetAddrFromStr(CTaskGetAddrFromStr *pSymbolInfo)
{
    char szBuffer[4096] = {0};
    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)szBuffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
    pSymbol->MaxNameLen = 512;
    pSymbol->ModBase = pSymbolInfo->m_dwBaseOfModule;

    if (SymFromNameW(GetSymbolHlpr()->m_hDbgProc, pSymbolInfo->m_wstrStr.c_str(), pSymbol))
    {
        pSymbolInfo->m_dwAddr = pSymbol->Address;
        return true;
    }
    return false;
}

bool CSymbolHlpr::SetSymbolPath(CTaskSetSymbolPath *pSymbolPath)
{
    return (TRUE == SymSetSearchPathW(GetSymbolHlpr()->m_hDbgProc, pSymbolPath->m_wstrSymbolPath.c_str()));
}

bool CSymbolHlpr::StackWalk(CTaskStackWalkInfo *pStackWalkInfo)
{
    //SymInitializeW(GetProcDbgger()->GetDbgProc(), /*L".;d:\\local\\StackWalker;d:\\local\\StackWalker\\x64\\Debug_VC9;C:\\Windows;C:\\Windows\\system32;SRV*C:\\websymbols*http://msdl.microsoft.com/download/symbols;"*/NULL, FALSE);
    //DWORD symOptions = SymGetOptions();  // SymGetOptions
    //symOptions |= SYMOPT_LOAD_LINES;
    //symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
    //symOptions |= SYMOPT_NO_PROMPTS;
    //SymSetOptions
    //symOptions = SymSetOptions(symOptions);
    //_GetModuleListTH32(GetProcDbgger()->GetDbgProc(), GetProcDbgger()->GetCurDbgProcId());

    STACKFRAME64 *pFrame = &(pStackWalkInfo->m_context);
    DWORD machineType = pStackWalkInfo->m_dwMachineType;
    int iMaxWalks = 1024;
    bool bStat = false;
    SetLastError(0);
    for (int i = 0 ; i < iMaxWalks ; i++)
    {
        //x86
#ifndef _WIN64
        BOOL bRet = ::StackWalk64(
            machineType,
            pStackWalkInfo->m_hDstProcess,
            pStackWalkInfo->m_hDstThread,
            pFrame,
            NULL,
            pStackWalkInfo->m_pfnReadMemoryProc,
            SymFunctionTableAccess64,
            pStackWalkInfo->m_pfnGetModuleBaseProc,
            pStackWalkInfo->m_pfnStackTranslateProc
            );
#else   //x64
        //DWORD dw = SymGetOptions();
        //dw &= ~SYMOPT_UNDNAME;
        //SymSetOptions(dw);

        BOOL bRet = ::StackWalk64(
            machineType,
            pStackWalkInfo->m_hDstProcess,
            pStackWalkInfo->m_hDstThread,
            pFrame,
            pStackWalkInfo->m_pThreadContext,
            NULL,
            NULL,
            NULL,
            NULL
            );
#endif
        if (bRet)
        {
            pStackWalkInfo->m_FrameSet.push_back(*pFrame);
            bStat = true;
        }
        else
        {
            int err = GetLastError();
            break;
        }
    }
    return bStat;
}

bool CSymbolHlpr::FindFileForDump(CTaskFindFileForDump *pFileInfo)
{
    WCHAR wszOut[MAX_PATH] = {0};
    SymFindFileInPathW(
        NULL,
        GetSymbolHlpr()->m_wstrSymbolPath.c_str(),
        pFileInfo->m_wstrModuleName.c_str(),
        (PVOID)pFileInfo->m_TimeStamp,
        (DWORD)pFileInfo->m_SizeofImage,
        0,
        SSRVOPT_DWORD,
        wszOut,
        NULL,
        NULL
        );
    pFileInfo->m_wstrFullPath = wszOut;
    return !pFileInfo->m_wstrFullPath.empty();
}

bool CSymbolHlpr::UnloadModuleTask(CTaskUnloadSymbol *pUnloadInfo)
{
    GetSymbolHlpr()->UnLoadModuleByAddr(pUnloadInfo->m_dwModuleAddr);
    return true;
}

bool CSymbolHlpr::UnloadAllModuleTask(LPVOID pParam)
{
    GetSymbolHlpr()->UnloadAllModules();
    return true;
}

bool CSymbolHlpr::DoTask(CSymbolTaskHeader *pTask)
{
    bool bStat = false;
    switch (pTask->m_eTaskType)
    {
    case  em_task_initsymbol:
        {
            bStat = InitEngine((CTaskSymbolInit *)pTask->m_pParam);
        }
        break;
    case  em_task_loadsym:
        {
            bStat = LoadSymbol((CTaskLoadSymbol *)pTask->m_pParam);
        }
        break;
    case  em_task_strfromaddr:
        {
            bStat = GetSymbolFromAddr((CTaskSymbolFromAddr *)pTask->m_pParam);
        }
        break;
    case  em_task_addrfromstr:
        {
            bStat = GetAddrFromStr((CTaskGetAddrFromStr *)pTask->m_pParam);
        }
        break;
    case  em_task_setpath:
        {
            bStat = SetSymbolPath((CTaskSetSymbolPath *)pTask->m_pParam);
        }
        break;
    case em_task_stackwalk:
        {
            bStat = StackWalk((CTaskStackWalkInfo *)pTask->m_pParam);
        }
        break;
    case em_task_findfile:
        {
            bStat = FindFileForDump((CTaskFindFileForDump *)pTask->m_pParam);
        }
        break;
    case em_task_unloadsym:
        {
            bStat = UnloadModuleTask((CTaskUnloadSymbol *)pTask->m_pParam);
        }
        break;
    case em_task_unloadall:
        {
            bStat = UnloadAllModuleTask(0);
        }
        break;
    }
    return bStat;
}

DWORD CSymbolHlpr::WorkThread(LPVOID pParam)
{
    CSymbolHlpr *pThis = (CSymbolHlpr *)pParam;
    CSymbolTaskHeader *pTask = NULL;

    //符号支持需要symsrv.dll的支持并需要symsrv.yes文件
    //忽略大小写匹配
    SymSetOptions(SymGetOptions() | SYMOPT_CASE_INSENSITIVE);
    //SymInitializeW(NULL, pThis->m_wstrSymbolPath.c_str(), FALSE);
    SetEvent(pThis->m_hInitEvent);
    while (TRUE)
    {
        WaitForSingleObject(pThis->m_hNotifyEvent, INFINITE);
        {
            CScopedLocker lock(pThis);
            if (pThis->m_vTaskQueue.empty())
            {
                continue;
            }
            pTask = *(pThis->m_vTaskQueue.begin());
            pThis->m_vTaskQueue.pop_front();
        }

        if (!pTask)
        {
            continue;
        }

        if (DoTask(pTask))
        {
            pTask->m_bSucc = TRUE;
        }
        SetEvent(pTask->m_hNotify);
    }
    return 0;
}

static CSymbolHlpr *s_ptr = NULL;

CSymbolHlpr *GetSymbolHlpr()
{
    return s_ptr;
}

bool InitSymbolHlpr(const ustring &wtrSymbolPath)
{
    s_ptr = new CSymbolHlpr(wtrSymbolPath);
    return true;
}