#include "ScriptHlpr.h"
#include <list>
#include "ScriptParser.h"
#include "StrUtil.h"

using namespace std;

unsigned int CScriptHlpr::msVarSerial = 0;

//ªÒ»°≈‰∂‘µƒ¿®∫≈Œª÷√ eg:type1 (, type2), type1 [, type2 ]
size_t CScriptHlpr::FindNextBracket(char type1, char type2, const mstring &str, size_t startPos) {
    list<char> stack;
    size_t pos1 = str.find(type1, startPos);
    size_t i = 0;

    stack.push_back(type1);
    for (i = pos1 + 1; i < str.size() ; i++)
    {
        char c = str[i];

        if (c == type1)
        {
            stack.push_back(type1);
        }

        if (c == type2)
        {
            char c2 = stack.back();
            if (c2 != type1)
            {
                throw(new CScriptParserException("¿®∫≈≈‰∂‘ ß∞‹"));
            }
            stack.pop_back();

            if (stack.empty())
            {
                return i;
            }
        }
    }
    throw(new CScriptParserException("¿®∫≈≈‰∂‘ ß∞‹"));
    return -1;
}

mstring CScriptHlpr::GetTempVarName() {
    return FormatA("@var_%d", msVarSerial++);
}