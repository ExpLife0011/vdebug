#pragma once
#include "mstring.h"

using namespace std;

class CScriptHlpr {
public:
    //查找配对括号位置
    static size_t FindNextBracket(char type1, char type2, const mstring &str, size_t startPos);
    //获取临时变量名称
    static mstring CScriptHlpr::GetTempVarName();

private:
    static unsigned int msVarSerial;
};