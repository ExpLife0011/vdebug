#ifndef PRINTFORMAT_H_H_
#define PRINTFORMAT_H_H_
#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include "mstring.h"

enum PrintFormatStat {
    line_start = 0,
    line_end,
    space
};

class PrintFormater {
public:
    PrintFormater();
    virtual ~PrintFormater();
    bool SetRule(const std::mstring &type);
    bool InitRule(const std::mstring &type, const std::mstring &rule);
    bool Reset();
    bool StartSession(const std::mstring &type);
    PrintFormater &operator << (const std::mstring &);
    PrintFormater &operator << (PrintFormatStat stat);
    bool EndSession();
    const char *GetResult();

private:
    std::map<std::mstring, std::vector<int>> m_FormatRule;
    std::vector<std::mstring> m_matrix1;
    std::vector<std::vector<std::mstring>> m_matrix2;
    static const int ms_space = 2;
};
#endif