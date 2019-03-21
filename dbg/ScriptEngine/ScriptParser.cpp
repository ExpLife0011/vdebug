#include "ScriptParser.h"
#include <ComLib/mstring.h>
#include <set>
#include <ComLib/StrUtil.h>
#include "ScriptHlpr.h"
#include "ScriptExpression.h"
#include "ScriptAccessor.h"
#include "../memory.h"
#include "../ProcDbg.h"

CScriptParser *CScriptParser::GetInst() {
    static CScriptParser *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CScriptParser();
    }
    return s_ptr;
}

void CScriptParser::init(CCmdBase *cmdEngine) {
    CScriptExpReader::GetInst()->InitReader();
    CScriptExpReader::GetInst()->SetCache(&mScriptCache);
    mCmdEngine = cmdEngine;
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

bool CScriptParser::RunLogic(LogicNode *root) {
    LogicNode *it = root;
    //run script
    VariateDesc *desc = NULL;
    ScriptCmdContext ctx;
    while (true) {
        if (it->mLogicType == em_logic_if || it->mLogicType == em_logic_elseif)
        {
            ctx = (*it->mCommandSet.begin());
            desc = CScriptExpReader::GetInst()->ParserExpression(ctx);
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
            for (list<ScriptCmdContext>::const_iterator ij = it->mCommandSet.begin() ; ij != it->mCommandSet.end() ; ij++)
            {
                ctx = *ij;
                CScriptExpReader::GetInst()->ParserExpression(ctx);
            }
            it = it->mNext;
        } else if (it->mLogicType == em_logic_end)
        {
            break;
        }
    }

    for (map<mstring, VariateDesc *>::const_iterator i2 = mScriptCache.mVarSet.begin() ; i2 != mScriptCache.mVarSet.end() ; i2++)
    {
        mstring name = i2->first;
        if (name.startwith("@var_"))
        {
            continue;
        }

        dp(L"%hs", i2->second->toString().c_str());
    }
    return true;
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

    CMemoryProc memoryOpt(CProcDbgger::GetInstance()->GetDbgProc());
    CScriptAccessor::GetInst()->SetContext(&memoryOpt, CProcDbgger::GetInstance());

    RunLogic(root);
    return true;
}

CScriptParser::CScriptParser() {
}

CScriptParser::~CScriptParser() {
}

void CScriptParser::ParserStrVar(mstring &script) const {
    size_t pos1 = 0, pos2 = 0;
    size_t lastPos = 0;
    bool unicode = false;
    while (true) {
        pos1 = script.find('"', lastPos);
        if (pos1 == mstring::npos)
        {
            break;
        }

        if (pos1 > 0 && script[pos1 - 1] == 'L')
        {
            unicode = true;
        }
        pos2 = script.find('"', pos1 + 1);
        if (pos2 == mstring::npos)
        {
            throw(new CScriptParserException("\"符号匹配失败"));
            return;
        }

        //忽略转义字符
        while (true) {
            if (pos2 > 0 && script[pos2 - 1] != '\\')
            {
                break;
            }

            pos2 = script.find('"', pos2 + 1);
            if (pos2 == mstring::npos)
            {
                throw(new CScriptParserException("\"符号匹配失败"));
                return;
            }
        }

        mstring content = script.substr(pos1 + 1, pos2 - pos1 - 1);
        VariateDesc *desc = NULL;
        if (unicode) {
            desc = CScriptExpReader::GetInst()->GetUnicodeDesc(content);
        } else {
            desc = CScriptExpReader::GetInst()->GetGbkDesc(content);
        }
        script.replace(pos1, pos2 - pos1 + 1, desc->mVarName);
        lastPos = pos1 + desc->mVarName.size();
    }
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

    ParserStrVar(script);
    //去除多余的空格和分隔符
    size_t i = 0;
    mstring tmp;
    bool flag1 = false;
    for (i = 0 ; i < script.size() ;)
    {
        char c = script[i];
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

LogicNode *CScriptParser::GetIfElseNode(const mstring &script, size_t &lastPos, LogicNode *&endNode) const {
    LogicNode *root = NULL;
    size_t pos1 = 0, pos2 = 0;
    mstring lastStr;
    LogicNode *nodeIf = new LogicNode();
    nodeIf->mLogicType = em_logic_if;

    pos1 = script.find('(', lastPos);
    pos2 = CScriptHlpr::FindNextBracket('(', ')', script, pos1);
    if (mstring::npos == pos2)
    {
        throw new CScriptParserException("if括号不配对");
        return NULL;
    }

    lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
    PushExpression(nodeIf, lastStr);

    if (root == NULL){
        root = nodeIf;
    }

    LogicNode *lastNode = new LogicNode();
    lastNode = nodeIf;
    endNode = new LogicNode();
    endNode->mLogicType = em_logic_end;

    pos1 = script.find('{', pos2);
    pos2 = CScriptHlpr::FindNextBracket('{', '}', script, pos1);

    if (mstring::npos == pos1 || mstring::npos == pos2) {
        throw (new CScriptParserException("{}匹配失败"));
    }

    lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
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

        PushExpression(elseIfNode, lastStr);

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

    if (NULL == lastNode->mRight)
    {
        lastNode->mRight = endNode;
    }

    //只有if 没有else if和else
    if (NULL == nodeIf->mRight)
    {
        nodeIf->mRight = endNode;
    }
    return root;
}

LogicNode *CScriptParser::GetWhileNode(const mstring &script, size_t &lastPos, LogicNode *&endNode) const {
    return NULL;
}

LogicNode *CScriptParser::GetForNode(const mstring &script, size_t &lastPos, LogicNode *&endNode) const {
    return NULL;
}

void CScriptParser::PushExpression(LogicNode *logicNode, const mstring &content, LogicNode *cmdLogic) const {
    ScriptCmdContext context;
    context.mCommand = content;
    context.mLogicRoot = cmdLogic;
    if (mstring::npos != content.find("bp "))
    {
        int dd = 1234;
    }

    CmdHandlerInfo cmdInfo;
    if (mCmdEngine->IsCommand(content, cmdInfo))
    {
        context.isDbggerCmd = true;
        context.mCmdType = cmdInfo.mCmdType;
    }

    logicNode->mCommandSet.push_back(context);
}

//返回逻辑节点起始位置
size_t CScriptParser::GetLogicStart(const mstring &script, size_t pos, mstring &startStr) const {
    size_t pos1 = mstring::npos, pos2 = mstring::npos;
    pos2 = script.find('{', pos);

    if (mstring::npos == pos2)
    {
        return mstring::npos;
    }

    for (size_t i = pos2 - 1 ; i != pos ; i--) {
        if (script[i] == ';')
        {
            pos1 = i;
            break;
        }
    }

    if (pos1 == mstring::npos)
    {
        pos1 = pos;
    } else {
        pos1 += 1;
    }
    startStr = script.substr(pos1, pos2 - pos1);
    return pos1;
}

LogicNode *CScriptParser::GetCommandNode(const mstring &command, const mstring &script, size_t &lastPos, LogicNode *&endNode) const {
    size_t pos1 = 0, pos2 = 0;
    mstring lastStr;
    LogicNode *nodeStart = NULL;

    pos1 = script.find('{', lastPos);
    pos2 = CScriptHlpr::FindNextBracket('{', '}', script, pos1);
    lastStr = script.substr(pos1 + 1, pos2 - pos1 - 1);
    nodeStart = new LogicNode();
    nodeStart->mLogicType = em_logic_order;

    //命令对应的独立的逻辑块
    LogicNode *end2 = new LogicNode();
    end2->mLogicType = em_logic_end;
    LogicNode *pp = GetLogicNode(lastStr, end2);
    PushExpression(nodeStart, command, pp);
    endNode = new LogicNode();
    endNode->mLogicType = em_logic_end;
    nodeStart->mNext = endNode;
    lastPos = pos2 + 1;
    return nodeStart;
}

//返回 逻辑块根节点
//参数 content，逻辑块字符串
//参数 endNode，该逻辑块的下一个节点或者结束
LogicNode *CScriptParser::GetLogicNode(const mstring &content, LogicNode *logicEnd) const {
    size_t lastPos = 0;
    LogicNode *root = NULL;
    LogicNode *newNode = NULL;
    mstring script = content;
    list<mstring>::const_iterator it;
    size_t pos1 = 0, pos2 = 0;
    mstring lastStr;
    LogicNode *endNode = NULL;

    size_t findPos = 0;
    while (true)
    {
        bool end = false;
        mstring startStr;
        findPos = GetLogicStart(script, lastPos, startStr);
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
                newNode = new LogicNode();
                if (root == NULL) {
                    root = newNode;
                }

                if (endNode && endNode->mEndPtr == NULL)
                {
                    endNode->mEndPtr = newNode;
                }

                for (it = set2.begin() ; it != set2.end() ; it++)
                {
                    PushExpression(newNode, *it);
                }
            }
        }

        if (end) {
            if (newNode) {
                newNode->mNext = logicEnd;
            } else {
                if (endNode) {
                    endNode->mEndPtr = logicEnd;
                } else {
                    root = new LogicNode();
                    root->mLogicType = em_logic_end;
                    root->mEndPtr = logicEnd;
                }
            }
            break;
        }

        //首次进行初始化操作
        if (endNode == NULL)
        {
            endNode = new LogicNode();
            endNode->mLogicType = em_logic_end;

            if (newNode)
            {
                newNode->mNext = endNode;
            }
        }

        lastPos = findPos;

        LogicNode *nodeStart = NULL;
        LogicNode *lastEnd = endNode;
        if (startStr.startwith("if(")) {
            lastEnd = endNode;
            nodeStart = GetIfElseNode(script, lastPos, endNode);
        }
        //while 循环
        else if (startStr.startwith("while(")) {
        }
        //for 循环
        else if (startStr.startwith("for(")){
        }
        //命令逻辑段
        else {
            nodeStart = GetCommandNode(startStr, script, lastPos, endNode);
        }

        if (lastEnd->mEndPtr == NULL) {
            lastEnd->mEndPtr = nodeStart;
        }

        if (root == NULL) {
            root = nodeStart;
        }
    }
    return root;
}

#include "../ProcCmd.h"

VOID WINAPI TestScript(HWND hwnd, HINSTANCE hinst, LPSTR cmd, int show) {
    /*
    ServSessionMgr *ptr = GetServSessionMgrInst();
    ptr->SessionStart("abcdef");
    ptr->SessionEnd("abcdef");
    */
    mstring aaa = "abf1";

    vector<int> intSet;
    intSet.push_back(1);
    intSet.push_back(2);
    intSet.insert(intSet.begin() + 2, 3);

    vector<int> dd = intSet;

    char path[256] = {0};
    GetModuleFileNameA(NULL, path, 256);
    PathAppendA(path, "..\\test.txt");

    PFILE_MAPPING_STRUCT pMapping = MappingFileA(path, FALSE, 1024 * 1024 * 8);
    mstring script = (const char *)pMapping->lpView;
    CloseFileMapping(pMapping);

    CProcCmd::GetInst()->InitProcCmd(CProcDbgger::GetInstance());
    CScriptParser::GetInst()->init(CProcCmd::GetInst());
    CScriptParser::GetInst()->parser(script);
    MessageBoxW(0, 0, 0, 0);
}