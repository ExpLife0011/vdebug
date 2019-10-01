#include "DbgInstall.h"
#include <Shlwapi.h>

using namespace std;

CDbgInstall *CDbgInstall::GetInst() {
    static CDbgInstall *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CDbgInstall();
    }
    return sPtr;
}

CDbgInstall::CDbgInstall() {
}

CDbgInstall::~CDbgInstall() {
}

bool CDbgInstall::InitInstall() {
    char buff[512];
#ifdef _DEBUG
    GetModuleFileNameA(NULL, buff, sizeof(buff));
    PathAppendA(buff, "..");
#else
    SHGetSpecialFolderPathW(NULL, buff, CSIDL_PROGRAM_FILES, TRUE);
    PathAppendA("vdebug");
#endif
    mInstallDir = buff;
    return true;
}

mstring CDbgInstall::GetInstallDir() const {
    return mInstallDir;
}

mstring CDbgInstall::GetSytleCfgPath() const {
    mstring path(mInstallDir);

    return path.path_append("StyleCfg.json");
}