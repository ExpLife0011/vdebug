#ifndef COMLIB_COMLIB_H_H_
#define COMLIB_COMLIB_H_H_
#include "json/json.h"
#include "ComUtil.h"
#include "tpool.h"
#include "StrSafe.h"
#include "LockBase.h"
#include "logger.h"
#include "crc32.h"
#include "winsize.h"
#include "sqlite3.h"
#include "SqliteOperator.h"
#include "DbProxy.h"
#include "TransferEncoder.h"
#include "servhlpr.h"
#include "SyntaxDef.h"

#ifndef COMLIB_EXPORTS
    #if _WIN64 || WIN64
        #ifdef _DEBUG
        #pragma comment(lib, "../Debug/ComLib64.lib")
        #else
        #pragma comment(lib, "../Release/ComLib64.lib")
        #endif //_DEBUG
    #else
        #ifdef _DEBUG
        #pragma comment(lib, "../Debug/ComLib32.lib")
        #else
        #pragma comment(lib, "../Release/ComLib32.lib")
        #endif //_DEBUG
    #endif //_WIN64
#endif //COMLIB_EXPORTS

#define TCP_PORT_MQ     6011
#endif //COMLIB_COMLIB_H_H_