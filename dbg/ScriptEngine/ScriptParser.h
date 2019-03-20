#pragma once
#include <string>
#include <ComLib/mstring.h>
#include "ScriptDef.h"

using namespace std;

class CScriptParserException {
public:
    CScriptParserException(const string &err) {
        mErr = err;
    }

    string GetError() {
        return mErr;
    }
private:
    string mErr;
};

class CScriptParser {
public:
    static CScriptParser *GetInst();
    void init();
    bool parser(const mstring &script);

private:
    CScriptParser();
    virtual ~CScriptParser();

private:
    //解析字符串逻辑块
    LogicNode *ParserStrNode(const mstring &content) const;
    //脚本串清洗
    void CleanStr(mstring &script) const;
    //将字符串以变量形式存储，防止对解析形成干扰
    void ParserStrVar(mstring &script) const;
    bool IsPartitionOpt(char c) const;
    //返回 逻辑块根节点
    //参数 content，逻辑块字符串
    //参数 endNode，该逻辑块的下一个节点或者结束
    LogicNode *GetLogicNode(const mstring &content, LogicNode *endNode) const;
    //Node集合节点折叠
    void AllNodeFold(LogicNode *root) const;
    //Node节点折叠
    void NodeFold(LogicNode *root) const;
    //返回逻辑节点起始位置
    size_t GetLogicStart(const mstring &script, size_t pos, mstring &startStr) const;

    LogicNode *GetIfElseNode(const mstring &script, size_t &lastPos, LogicNode *&endNode) const;
    LogicNode *GetCommandNode(const mstring &command, const mstring &script, size_t &lastPos, LogicNode *&endNode) const;
    LogicNode *GetWhileNode(const mstring &script, size_t &lastPos, LogicNode *&endNode) const;
    LogicNode *GetForNode(const mstring &script, size_t &lastPos, LogicNode *&endNode) const;
private:
    list<VariateDesc> mGlobalVar;
    list<FunctionDesc> mGlobalFun;
    ScriptCache mScriptCache;
};