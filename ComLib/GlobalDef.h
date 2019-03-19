#ifndef GLOBALDEF_H_H_
#define GLOBALDEF_H_H_
#include <Windows.h>
#include <Shlwapi.h>
#include "mstring.h"

#define SFV_SERVICE_NAME           "DbgService"
#define SFV_SERVICE_DISPLAY_NAME   "DbgService"
#define SFV_SERVICE_DESCRIPTION    "DbgService服务"

#define SFV_NOTIFY_NAME "Global\\{784BC5BC-25D1-4861-8FED-38CFF9428877}"

#define PATH_SERVICE_CACHE "software\\vdebug\\runner"
#define RUNNER_EVENT32  ("dbg32_%hs")
#define RUNNER_EVENT64  ("dbg64_%hs")
#define SERVICE_EVENT   ("service_%hs")

#define DBG_CTRL_ERROR_SUCCESS          (0)
#define DBG_CTRL_ERROR_CMD_NOTFIND      (6001)
#define DBG_CTRL_ERROR_CMD_SYNTAX_ERR   (6002)

#define DBG_CMD_SYNTAX_ERR              (9050)
#define DBG_CMD_READMEM_ERR             (9051)
#define DBG_CMD_UNKNOW_ERR              (9999)

#if WIN64 || _WIN64
#define REG_VDEBUG_CACHE    "SoftWare\\WOW6432Node\\vdebug\\config\\dbgport"
#define REG_VDEBUG_STATUS   "SoftWare\\WOW6432Node\\vdebug\\dbg\\status"
#define REG_VDEBUG_DESC     "SoftWare\\WOW6432Node\\vdebug\\desc"
#else
#define REG_VDEBUG_CACHE    "SoftWare\\vdebug\\config\\dbgport"
#define REG_VDEBUG_STATUS   "SoftWare\\vdebug\\dbg\\status"
#define REG_VDEBUG_DESC     "SoftWare\\vdebug\\desc"
#endif

#define UNIQUE_DEBUG    "debug"    //调试用

struct ProcMonInfo {
    DWORD procUnique;
    DWORD procPid;
    std::mstring procPath;
    std::mstring procCmd;
    BOOL x64;
    std::mstring startTime;
    DWORD sessionId;
    DWORD parentPid;
    std::mstring procDesc;
    std::mstring procUser;
    std::mstring procUserSid;

    ProcMonInfo() {
        procUnique = 0;
        procPid = 0;
        sessionId = 0;
        x64 = FALSE;
        parentPid = 0;
    }
};

static std::mstring GetBaseDir() {
#if WIN64 || _WIN64
    HMODULE m = GetModuleHandleA("ComLib64.dll");
#else
    HMODULE m = GetModuleHandleA("ComLib32.dll");
#endif

    char buffer[512];
    GetModuleFileNameA(m, buffer, 512);
    PathAppendA(buffer, "..");
    return buffer;
}

static std::mstring GetConfigDbPath() {
    return (GetBaseDir() + "\\db\\cfg.db");
}

static std::mstring GetSymbolDbPath() {
    return (GetBaseDir() + "\\db\\symbol.db");
}
#endif //GLOBALDEF_H_H_