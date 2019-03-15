#pragma once
#include <Windows.h>
#include <list>
#include <string>

using namespace std;

enum VariateType {
    em_var_pending,     //未决类型的
    em_var_str_gbk,     //多字节字符串
    em_var_str_unicode, //宽字符串
    em_var_int,         //整形64位
    em_var_ptr          //指针类型
};

struct VariateDesc {
    VariateType mVarType;
    int mVarLength;
    mstring mVarName;

    mstring mStrValue;      //gbk/unicode
    DWORD64 mIntValue;      //Int64
    LPVOID mPtrValue;       //ptr value

    VariateDesc &operator=(const VariateDesc &other) {
        mVarType = other.mVarType;
        mVarLength = other.mVarLength;
        mStrValue = other.mStrValue;
        mIntValue = other.mIntValue;
        mPtrValue = other.mPtrValue;
        return *this;
    }

    VariateDesc () {
        mVarType = em_var_pending;
        mVarLength = 0;
        mIntValue = 0;
        mPtrValue = NULL;
    }
};

//expression
enum ExpressionType {
    em_expression_add,      //+
    em_expression_sub,      //-
    em_expression_mult,     //*
    em_expression_div,      ///

    em_expression_and,      //&&
    em_expression_or,       //||

    em_expression_bit_and,  //&
    em_expression_bit_or,   //|
    em_expression_value,    //[]
};

enum ExpressNodeType {
    em_expnode_fun,
    em_expnode_express,
    em_expnode_command
};

struct FunctionDesc {
    bool mInternal;
    string mFunName;
    list<VariateDesc> mParamSet;
};

// 1434534 + 123 * (1111 + 3344);bp;strlen("abcdef")
struct ExpressionNode {
    ExpressNodeType mType;
    //express content
    mstring mContent;

    // for express
    mstring mExpress;

    // for function
    FunctionDesc mFunction;

    // for command
    mstring mCommand;
};

enum LogicNodeType {
    em_logic_order,     //order
    em_logic_if,        //if
    em_logic_elseif,    //else if
    em_logic_end        //end node
};

struct LogicNode {
    LogicNodeType mLogicType;   //logic type

    LogicNode *mNext;           //logic for logic order

    LogicNode *mSub;            //sub logic
    LogicNode *mParent;         //parent logic

    LogicNode *mBrotherFront;   //front brother
    LogicNode *mBrotherNext;    //next brother

    LogicNode *mLeft;           //logic for right eg:if, else if, 
    LogicNode *mRight;          //logic for wrong

    LogicNode *mEndPtr;         //end类型的指针对应的对象
    list<mstring> mCommandSet;  //commmand set for parser.

    LogicNode() {
        mLogicType = em_logic_order;
        mNext = NULL;
        mSub = NULL;
        mParent = NULL;
        mLeft = NULL;
        mRight = NULL;
        mBrotherFront = NULL;
        mBrotherNext = NULL;
        mEndPtr = NULL;
    }
};

struct ScriptCache {
    list<ExpressionNode *> mExpressionSet;    //expression list
    list<VariateDesc *> mVarSet;              //var set
    list<FunctionDesc *> mFunSet;             //function set

    VariateDesc *GetVarByName(const mstring &name) {
        return NULL;
    }

    FunctionDesc *GetFunctionByName(const mstring &name) {
        return NULL;
    }
};