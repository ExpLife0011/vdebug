#pragma once
#include <Windows.h>
#include "../SyntaxHlpr/SyntaxCache.h"
#include "StyleConfigMgr.h"

class CCmdShowView : public CSyntaxCache {
public:
    CCmdShowView();
    virtual ~CCmdShowView();

    bool InitShowView();
    bool LoadUserCfg(const CStyleConfig &cfg);
private:
    static void __stdcall SendDefaultParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *s,
        void *param
        );
    static void __stdcall RecvDefaultParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *s,
        void *param
        );
};