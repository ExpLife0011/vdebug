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
    //�����ַ����߼���
    LogicNode *ParserStrNode(const mstring &content) const;
    //�ű�����ϴ
    void CleanStr(mstring &script) const;
    //���ַ����Ա�����ʽ�洢����ֹ�Խ����γɸ���
    void ParserStrVar(mstring &script) const;
    bool IsPartitionOpt(char c) const;
    //���� �߼�����ڵ�
    //���� content���߼����ַ���
    //���� endNode�����߼������һ���ڵ���߽���
    LogicNode *GetLogicNode(const mstring &content, LogicNode *endNode) const;
    //Node���Ͻڵ��۵�
    void AllNodeFold(LogicNode *root) const;
    //Node�ڵ��۵�
    void NodeFold(LogicNode *root) const;
    //�����߼��ڵ���ʼλ��
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