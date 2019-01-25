#ifndef TRANSFERENCODER_COMLIB_H_H_
#define TRANSFERENCODER_COMLIB_H_H_
#include <Windows.h>
#include <list>
#include <ComStatic/ComStatic.h>
#include <dbg/TitanEngine/TitanEngine.h>

struct ProcInfoSet {
    std::list<ProcMonInfo> mAddSet;
    std::list<DWORD> mKillSet;
};
std::mstring _declspec(dllexport) __stdcall EncodeProcMon(const ProcInfoSet &procSet);
ProcInfoSet _declspec(dllexport) __stdcall DecodeProcMon(const std::mstring &json);

struct ProcCreateInfo {
    DWORD mPid;
    std::mstring mImage;
    std::mstring mBaseAddr;
    std::mstring mEntryAddr;
};
std::mstring _declspec(dllexport) __stdcall EncodeProcCreate(const ProcCreateInfo &info);
ProcCreateInfo _declspec(dllexport) __stdcall DecodeProcCreate(const std::mstring &json);

struct DllLoadInfo {
    std::mstring mDllName;
    std::mstring mBaseAddr;
    std::mstring mEndAddr;
};
std::mstring _declspec(dllexport) __stdcall EncodeDllLoadInfo(const DllLoadInfo &info);
DllLoadInfo _declspec(dllexport) __stdcall DecodeDllLoadInfo(const std::mstring &json);

struct RegisterContent {
    TITAN_ENGINE_CONTEXT_t mContext;
    std::mstring mCipStr;
};
std::mstring _declspec(dllexport) __stdcall EncodeCmdRegister(const RegisterContent &context);
RegisterContent _declspec(dllexport) __stdcall DecodeCmdRegister(const std::mstring &json);

struct CallStackSingle {
    std::mstring mReturn;
    std::mstring mParam0;
    std::mstring mParam1;
    std::mstring mParam2;
    std::mstring mParam3;
    std::mstring mAddr;
    std::mstring mFunction;
};

struct CallStackData {
    std::list<CallStackSingle> mCallStack;
};
std::mstring _declspec(dllexport) __stdcall EncodeCmdCallStack(const CallStackData &callStack);
CallStackData _declspec(dllexport) __stdcall DecodeCmdCallStack(const std::mstring &json);
#endif //TRANSFERENCODER_H_H_