#pragma once
#include <Windows.h>
#include <list>
#include <map>
#include <string>
#include "StrUtil.h"

using namespace std;

enum VariateType {
    em_var_pending,     //未决类型的
    em_var_str_gbk,     //多字节字符串
    em_var_str_unicode, //宽字符串
    em_var_int,         //整形64位
    em_var_ptr,         //指针类型
    em_var_double       //浮点型数据
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

    mstring toString() {
        mstring str;
        switch (mVarType) {
            case em_var_int:
                str += FormatA("var %hs, type int, value %llu", mVarName.c_str(), mIntValue);
                break;
            case em_var_str_gbk:
            case em_var_str_unicode:
                str += FormatA("var %hs, type str, value %hs", mVarName.c_str(), mStrValue.c_str());
                break;
            case em_var_ptr:
                str += FormatA("var %hs, type ptr, value %p", mVarName.c_str(), mPtrValue);
                break;
            case em_var_pending:
                str += FormatA("var %hs, type pending", mVarName.c_str());
                break;
            default:
                break;
        }
        return str;
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
    list<VariateDesc *> mParamSet;
    VariateDesc *mReturn;

    FunctionDesc() {
        mInternal = true;
        mReturn = NULL;
    }
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
    map<mstring, VariateDesc *> mVarSet;              //var set

    VariateDesc *GetVarByName(const mstring &name) {
        map<mstring, VariateDesc *>::const_iterator it = mVarSet.find(name);

        if (it != mVarSet.end())
        {
            return it->second;
        }
        return NULL;
    }

    void InsertVar(VariateDesc *desc) {
        mVarSet[desc->mVarName] = desc;
    }
};