#ifndef DBGCTRL_DBGCTRL_H_H_
#define DBGCTRL_DBGCTRL_H_H_

#if _WIN64 || WIN64
    #ifdef _DEBUG
        #pragma comment(lib, "../Debug/x64/DbgCtrl64.lib")
    #else
        #pragma comment(lib, "../Release/x64/DbgCtrl64.lib")
    #endif //_DEBUG
#else
    #ifdef _DEBUG
        #pragma comment(lib, "../Debug/x32/DbgCtrl32.lib")
    #else
        #pragma comment(lib, "../Release/x32/DbgCtrl32.lib")
    #endif //_DEBUG
#endif //_WIN64

#include "DbgService.h"
#include "DbgClient.h"
#include "DbgCtrlCom.h"
#endif //DBGCTRL_DBGCTRL_H_H_