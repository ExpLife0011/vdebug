#include <Windows.h>
#include <shlobj.h>
#include <ComStatic/ComStatic.h>
#include "DbProxy.h"

using namespace std;

DbProxy::DbProxy() {
}

DbProxy::~DbProxy() {
}

DbProxy *DbProxy::GetInstance() {
    static DbProxy *s_ptr = NULL;
    if (s_ptr == NULL)
    {
        s_ptr = new DbProxy();
    }

    return s_ptr;
}

bool DbProxy::InitDbEnv() {
    mCfgDbPath = GetConfigDbPath();
    mSymbolDbPath = GetSymbolDbPath();

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(mCfgDbPath.c_str()))
    {
        char dir[512];
        lstrcpynA(dir, mCfgDbPath.c_str(), 512);
        PathAppendA(dir, "..");
        SHCreateDirectoryExA(NULL, dir, NULL);
    }

    SqliteOperator opt(mCfgDbPath);
    //tOpenHistory id path param dir time Dbgger cache
    opt.Exec("create table if not exists tOpenHistory(id BIGINT PRIMARY KEY, path CHAR(512), param TEXT, dir TEXT, time CHAR(32))");
    return true;
}

SqliteResult DbProxy::SelectCfg(const mstring &sql) {
    SqliteOperator opt(mCfgDbPath);
    return opt.Select(sql);
}

bool DbProxy::ExecCfg(const mstring &sql) {
    SqliteOperator opt(mCfgDbPath);
    return opt.Exec(sql);
}

SqliteResult DbProxy::SelectSymbol(const mstring &sql) {
    SqliteOperator opt(mSymbolDbPath);
    return opt.Select(sql);
}

bool DbProxy::ExecSymbol(const mstring &sql) {
    SqliteOperator opt(mSymbolDbPath);
    return opt.Exec(sql);
}