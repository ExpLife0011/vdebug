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
    //设置格式化规则,格式:0;0;15;0
    bool SetRule(const std::mstring &type);
    bool Reset();
    bool StartSession(const std::mstring &type);
    PrintFormater &operator << (const std::mstring &);
    PrintFormater &operator << (PrintFormatStat stat);
    bool EndSession();
    std::mstring GetResult();
    std::vector<std::mstring> GetResultSet();
    void SetSpaceCount(int count);

private:
    std::vector<int> m_rule;
    std::vector<std::mstring> m_matrix1;
    std::vector<std::vector<std::mstring>> m_matrix2;
    int m_space;
};
#endif