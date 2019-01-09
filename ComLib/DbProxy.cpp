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
    mCfgOpt.Open(mCfgDbPath);
    SqliteResult res = mCfgOpt.Select(sql);
    mCfgOpt.Close();
    return res;
}

bool DbProxy::ExecCfg(const mstring &sql) {
    mCfgOpt.Open(mCfgDbPath);
    bool ret = mCfgOpt.Exec(sql);
    mCfgOpt.Close();
    return ret;
}

SqliteResult DbProxy::SelectSymbol(const mstring &sql) {
    mSymbolOpt.Open(mSymbolDbPath);
    SqliteResult res = mSymbolOpt.Select(sql);
    mSymbolOpt.Close();
    return res;
}

bool DbProxy::ExecSymbol(const mstring &sql) {
    mSymbolOpt.Open(mSymbolDbPath);
    bool ret = mSymbolOpt.Exec(sql);
    mSymbolOpt.Close();
    return ret;
}