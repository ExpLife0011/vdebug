#pragma once
#include <Windows.h>
#include "../../ComLib/mstring.h"

class CDbgInstall {
public:
    static CDbgInstall *GetInst();
    bool InitInstall();
    std::mstring GetInstallDir() const;
    std::mstring GetSytleCfgPath() const;

private:
    CDbgInstall();
    virtual ~CDbgInstall();

private:
    std::mstring mInstallDir;
};