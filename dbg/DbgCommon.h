#ifndef DBGCOMMON_DBG_H_H_
#define DBGCOMMON_DBG_H_H_
#include <Windows.h>
#include <ComLib/mstring.h>

class CDbgCommon {
public:
    static std::mstring GetSystemStr(DWORD majver, DWORD minver, DWORD product);
    static std::mstring GetExceptionDesc(DWORD code);
    static std::mstring GetProcSymFromAddr(DWORD64 dwAddr, const std::mstring &dllName, DWORD64 moduleBase);
    static std::mstring GetSymFromAddr(DWORD64 dwAddr, const std::mstring &dllName, DWORD64 moduleBase);
};
#endif //DBGCOMMON_DBG_H_H_