#pragma once
#include "mstring.h"

using namespace std;

class CScriptHlpr {
public:
    //�����������λ��
    static size_t FindNextBracket(char type1, char type2, const mstring &str, size_t startPos);
};