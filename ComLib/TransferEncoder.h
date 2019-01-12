#ifndef TRANSFERENCODER_COMLIB_H_H_
#define TRANSFERENCODER_COMLIB_H_H_
#include <Windows.h>
#include <list>
#include <ComStatic/ComStatic.h>

struct ProcInfoSet {
    std::list<ProcMonInfo> mAddSet;
    std::list<DWORD> mKillSet;
};
std::utf8_mstring _declspec(dllexport) __stdcall EncodeProcMon(const ProcInfoSet &procSet);
ProcInfoSet _declspec(dllexport) __stdcall DecodeProcMon(const std::utf8_mstring &json);

struct ProcCreateInfo {
    DWORD mPid;
    std::ustring mImage;
    std::ustring mBaseAddr;
    std::ustring mEntryAddr;
};
std::utf8_mstring _declspec(dllexport) __stdcall EncodeProcCreate(const ProcCreateInfo &info);
ProcCreateInfo _declspec(dllexport) __stdcall DecodeProcCreate(const std::utf8_mstring &json);

struct DllLoadInfo {
    std::ustring mDllName;
    std::ustring mBaseAddr;
    std::ustring mEndAddr;
};
std::utf8_mstring _declspec(dllexport) __stdcall EncodeDllLoadInfo(const DllLoadInfo &info);
DllLoadInfo _declspec(dllexport) __stdcall DecodeDllLoadInfo(const std::utf8_mstring &json);
#endif //TRANSFERENCODER_H_H_