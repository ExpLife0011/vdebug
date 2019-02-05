#ifndef DBPROXY_STATIC_H_H_
#define DBPROXY_STATIC_H_H_
#include <ComStatic/mstring.h>
#include "SqliteOperator.h"
#include "LibDef.h"

class COMAPI DbProxy {
private: 
    DbProxy();

public:
    virtual ~DbProxy();
    static DbProxy *GetInstance();
    bool InitDbEnv();
    SqliteResult SelectCfg(const std::mstring &sql);
    bool ExecCfg(const std::mstring &sql);
    SqliteResult SelectSymbol(const std::mstring &sql);
    bool ExecSymbol(const std::mstring &sql);

private:
    std::mstring mCfgDbPath;
    std::mstring mSymbolDbPath;
    SqliteOperator mCfgOpt;
    SqliteOperator mSymbolOpt;
};
#endif //DBPROXY_STATIC_H_H_