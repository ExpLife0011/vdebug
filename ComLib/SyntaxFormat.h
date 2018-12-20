#ifndef PRINTFORMAT_H_H_
#define PRINTFORMAT_H_H_
#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include "ComUtil.h"

using namespace std;

class SyntaxFormater : public PrintFormater {
public:
    SyntaxFormater();
    virtual ~SyntaxFormater();

    virtual bool InitRule(const char *type, const char *rule);
    virtual bool SetRule(const char *type);
    virtual bool Reset();

    virtual PrintFormater &operator << (const char*);
    virtual PrintFormater &operator << (PrintFormatStat stat);

    virtual bool StartSession(const char *type);
    virtual bool EndSession();
    virtual const char *GetResult();
private:
    map<string, vector<int>> m_FormatRule;
    vector<string> m_matrix1;
    vector<vector<string>> m_matrix2;
    static const int ms_space = 2;
};
#endif