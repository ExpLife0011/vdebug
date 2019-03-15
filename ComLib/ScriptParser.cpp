#include "ScriptParser.h"
#include "mstring.h"
#include <set>
#include "StrUtil.h"
#include "ScriptHlpr.h"
#include "ScriptExpression.h"

CScriptParser *CScriptParser::GetInst() {
    static CScriptParser *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CScriptParser();
    }
    return s_ptr;
}

void CScriptParser::init() {
    CScriptExpReader::GetInst()->InitReader();
}

void CScriptParser::NodeFold(LogicNode *root) const {
    LogicNode *tmp1 = NULL;
    LogicNode *tmp2 = NULL;
    if (root->mLogicType == em_logic_if || root->mLogicType == em_logic_elseif)
    {
        tmp1 = root;
        tmp2 = root->mLeft;
        while (tmp2->mLogicType == em_logic_end && tmp2->mEndPtr != NULL) {
            tmp1->mLeft = tmp2->mEndPtr;
            tmp2 = tmp2->mEndPtr;
        }

        tmp2 = root->mRight;
        while (tmp2->mLogicType == em_logic_end && tmp2->mEndPtr != NULL) {
            tmp1->mRight = tmp2->mEndPtr;
            tmp2 = tmp2->mEndPtr;
        }
    } else if (root->mLogicType == em_logic_order)
    {
        tmp1 = root;
        tmp2 = root->mNext;
        while (tmp2->mLogicType == em_logic_end && tmp2->mEndPtr != NULL) {
            tmp1->mNext = tmp2->mEndPtr;
            tmp2 = tmp2->mEndPtr;
        }
    }
}

void CScriptParser::AllNodeFold(LogicNode *root) const {
    list<LogicNode *> set1;

    if (root->mLogicType != em_logic_end)
    {
        set1.push_back(root);
    }

    //EndNode折叠
    while (!set1.empty()) {
        LogicNode *tmpNode = set1.front();
        set1.pop_front();

        NodeFold(tmpNode);
        switch (tmpNode->mLogicType)
        {
        case em_logic_order: 
            {
                if (tmpNode->mNext->mLogicType != em_logic_end)
                {
                    set1.push_back(tmpNode->mNext);
                }
            }
            break;
        case em_logic_if:
        case em_logic_elseif:
            {
                if (tmpNode->mRight->mLogicType != em_logic_end)
                {
                    set1.push_back(tmpNode->mRight);
                }

                if (tmpNode->mLeft->mLogicType != em_logic_end)
                {
                    set1.push_back(tmpNode->mLeft);
                }
            }
            break;
        default:
            throw new CScriptParserException("未定义的逻辑类型");
        }
    }
}

bool CScriptParser::parser(const mstring &script) {
    mstring tmp = script;
    CleanStr(tmp);
    tmp.trim();

    LogicNode *endNode = new LogicNode();
    endNode->mLogicType = em_logic_end;
    LogicNode *root = GetLogicNode(tmp, endNode);
    AllNodeFold(root);

    LogicNode *it = root;
    CScriptExpReader::GetInst()->SetCache(&mScriptCache);

    //run script
    VariateDesc *desc = NULL;
    while (true) {
        if (it->mLogicType == em_logic_if || it->mLogicType == em_logic_elseif)
        {
            desc = CScriptExpReader::GetInst()->ParserExpression(*(it->mCommandSet.begin()));
            if (desc->mVarType != em_var_int)
            {
                throw (new CScriptParserException("if语句执行错误"));
                break;
            }

            if (desc->mIntValue == 1)
            {
                it = it->mLeft;
            } else {
                it = it->mRight;
            }
        } else if (it->mLogicType == em_logic_order)
        {
            for (list<mstring>::const_iterator ij = it->mCommandSet.begin() ; ij != it->mCommandSet.end() ; ij++)
            {
                CScriptExpReader::GetInst()->ParserExpression(*ij);
            }
            it = it->mNext;
        } else if (it->mLogicType == em_logic_end)
        {
            break;
        }
    }
    return true;
}

CScriptParser::CScriptParser() {
}

CScriptParser::~CScriptParser() {
}

bool CScriptParser::IsPartitionOpt(char c) const {
    if (c == 0x00 || c == '\r' || c == '\n' || c == '\t' || c == ' ')
    {
        return true;
    }
    return false;
}

//清理脚本串,方便模式匹配
//bp kernel32! createfilew  ; if (!endwith(str([$esp+4]), "test4.txt")) { g }
//bp kernel32!createfilew;if(!endwith(str([$esp+4]),"test4.txt")){g;}
void CScriptParser::CleanStr(mstring &script) const {
    //去除注释，多余的分割符，多余的空格
    size_t pos1 = 0, pos2 = 0, pos3 = 0;

    //删除 // /**/格式的注释
    while (true) {
        pos1 = script.find("/*");
        pos2 = script.find("//");

        if (mstring::npos == pos1 && mstring::npos == pos2){
            break;
        }
        else if (mstring::npos != pos1) {
            pos3 = script.find("*/", pos1);
            if (mstring::npos == pos3)
            {
                throw (new CScriptParserException("注释没有对应的结束标识"));
                return;
            }

            script.erase(pos1, pos3 - pos1 + 2);
        } else if (mstring::npos != pos2) {
            pos3 = script.find('\n', pos2);
            if (mstring::npos != pos3){
                script.erase(pos2, pos3 - pos2 + 1);
            } else {
                script.erase(pos2, script.size() - pos2);
            }
        } else {
            if (pos1 < pos2) {
                pos3 = script.find("*/", pos1);
                if (mstring::npos == pos3)
                {
                    throw (new CScriptParserException("注释没有对应的结束标识"));
                    return;
                }

                script.erase(pos1, pos3 - pos1 + 2);
            } else {
                pos3 = script.find('\n', pos2);
                if (mstring::npos != pos3){
                    script.erase(pos2, pos3 - pos2 + 1);
                } else {
                    script.erase(pos2, script.size() - pos2);
                }
            }
        }
    }

    //去除多余的空格和分隔符
    size_t i = 0;
    mstring tmp;
    bool flag1 = false;
    for (i = 0 ; i < script.size() ;)
    {
        char c = script[i];
        if (c == '"') {
            size_t t = script.find('"', i + 1);
            if (mstring::npos == t)
            {
                throw (new CScriptParserException("字符串格式错误"));
                return;
            }
            tmp += script.substr(i, t - i + 1);
            i = t + 1;
            continue;
        }

        if (c == '\r' || c == '\n' || c == '\t' || c == ' '){
            if (flag1){
            } else {
                flag1 = true;
                tmp += " ";
            }
        } else {
            flag1 = false;
            tmp += c;
        }
        i++;
    }
    script = tmp;

    //去除特殊符号前后空格( ) + - * / [ ] . !
    static set<char> sOpt;
    if (sOpt.empty()){
        sOpt.insert('('), sOpt.insert(')'), sOpt.insert('+');
        sOpt.insert('-'), sOpt.insert('*'), sOpt.insert('/');
        sOpt.insert('['), sOpt.insert(']'), sOpt.insert('.');
        sOpt.insert('!'), sOpt.insert('='), sOpt.insert(';');
        sOpt.insert('{'), sOpt.insert('}'), sOpt.insert('>');
        sOpt.insert('='), sOpt.insert('%'), sOpt.insert('^');
        sOpt.insert('!'), sOpt.insert('<'), sOpt.insert('|');
        sOpt.insert(',');
    }

    tmp.clear();
    flag1 = false;
    size_t j = 0;
    for (i = 0 ; i < script.size() ;) {
        char c = script[i];

        if (c == '"') {
            j = script.find('"', i + 1);
            tmp += script.substr(i, j - i + 1);
            i = j + 1;
            continue;
        }

        if (sOpt.end() != sOpt.find(c)) {
            tmp.trimright();
            tmp += c;
            i++;

            if (script[i] == ' ')
            {
                i++;
            }
            continue;
        }
        tmp += c;
        i++;
    }
    script = tmp;
}

//返回 逻辑块根节点
//参数 content，逻辑块字符串
//参数 endNode，该逻辑块的下一个节点或者结束
LogicNode *CScriptParser::GetLogicNode(const mstring &content, LogicNode *logicEnd) const {
    size_t lastPos = 0;
    LogicNode *root = NULL;
    LogicNode *newNode = NULL;
    LogicNode *lastNode = NULL;
    mstring script = content;
    list<mstring>::const_iterator it;
    size_t pos1 = 0, pos2 = 0;
    mstring lastStr;
    LogicNode *endNode = NULL;

    size_t findPos = 0;
    while (true)
    {
        bool end = false;
        findPos = script.find("if(", lastPos);
        pos1 = lastPos;
        if (mstring::npos == findPos) {
            end = true;
            pos2 = script.size();
        } else {
            pos2 = findPos;
        }

        newNode = NULL;
        if (pos2 > pos1 + 1) {
            lastStr = script.substr(pos1, pos2 - pos1);
            list<mstring> set2 = SplitStrA(lastStr, ";");
            if (set2.size()) {
                if (root == NULL) {
                    root = new LogicNode();
                    newNode = root;
                } else {
                    newNode = new LogicNode();
                }

                if (endNode && endNode->mEndPtr == NULL)
                {
                    endNode->mEndPtr = newNode;
                }

                for (it = set2.begin() ; it != set2.end() ; it++)
                {
                    newNode->mCommandSet.push_back(*it);
                }
                lastNode = newNode;
            }
        }

        if (end) {
            if (newNode) {
                newNode->mNext = logicEnd;
            } else {
                if (endNode) {
                    endNode->mEndPtr = logicEnd;
                } else {
                    LogicNode *empty = new LogicNode();
                    empty->mLogicType = em_logic_end;
                    empty->mEndPtr = logicEnd;
                    return empty;
                }
            }
            break;
        }

        lastPos = findPos;
        if (0 == strncmp(script.c_str() + lastPos, "if(", strlen("if("))) {
            LogicNode *nodeIf = new LogicNode();
            nodeIf->mLogicType = em_logic_if;

            pos1 = script.find('(', lastPos);
            pos2 = CScriptHlpr::FindNextBracket('(', ')', script, pos1);
            if (mstring::npos == pos2)
            {
                throw new CScriptParserException("if括号不配对");
                break;
            }

            lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
            nodeIf->mCommandSet.push_back(lastStr);

            if (root == NULL){
                root = nodeIf;
                lastNode = nodeIf;
            } else {
                lastNode->mBrotherNext = nodeIf;
                nodeIf->mBrotherFront = lastNode;
                lastNode->mNext = nodeIf;
            }

            if (endNode && !endNode->mEndPtr)
            {
                endNode->mEndPtr = nodeIf;
            }
            endNode = new LogicNode();
            endNode->mLogicType = em_logic_end;

            pos1 = script.find('{', pos2);
            pos2 = CScriptHlpr::FindNextBracket('{', '}', script, pos1);

            if (mstring::npos == pos1 || mstring::npos == pos2) {
                throw (new CScriptParserException("{}匹配失败"));
            }

            lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
            //此处有逻辑错误
            LogicNode *nodeIfSub = GetLogicNode(lastStr, endNode);

            nodeIf->mLeft = nodeIfSub;
            //right set to end node
            nodeIf->mRight = NULL;
            lastPos = pos2 + 1;

            lastNode = nodeIf;
            while (true) {
                if (0 != strncmp(script.c_str() + lastPos, "else if(", strlen("else if("))) {
                    break;
                }

                pos1 = script.find('(', lastPos);
                pos2 = CScriptHlpr::FindNextBracket('(', ')', script, pos1);
                if (mstring::npos == pos1 || mstring::npos == pos2)
                {
                    throw (new CScriptParserException("{}匹配失败"));
                }

                LogicNode *elseIfNode = new LogicNode();
                elseIfNode->mLogicType = em_logic_elseif;
                lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
                elseIfNode->mCommandSet.push_back(lastStr);

                pos1 = script.find('{', pos2);
                pos2 = CScriptHlpr::FindNextBracket('{', '}', script, pos1);

                if (mstring::npos == pos1 || mstring::npos == pos2) {
                    throw (new CScriptParserException("{}匹配失败"));
                }

                lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
                LogicNode *nodeIfElseSub = GetLogicNode(lastStr, endNode);
                elseIfNode->mLeft = nodeIfElseSub;

                if (lastNode)
                {
                    lastNode->mRight = elseIfNode;
                    lastNode = elseIfNode;
                }
                lastPos = pos2 + 1;
            }

            if (0 == strncmp(script.c_str() + lastPos, "else{", strlen("else{"))) {
                pos1 = script.find('{', lastPos);
                pos2 = CScriptHlpr::FindNextBracket('{', '}', script, pos1);

                if (mstring::npos == pos1 || mstring::npos == pos2) {
                    throw (new CScriptParserException("{}匹配失败"));
                }

                lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
                LogicNode *elseSubNode = GetLogicNode(lastStr, endNode);
                lastNode->mRight = elseSubNode;
                lastPos = pos2 + 1;
            }

            //只有if 没有else if和else
            if (NULL == nodeIf->mRight)
            {
                nodeIf->mRight = endNode;
            }
        }
    }
    return root;
}