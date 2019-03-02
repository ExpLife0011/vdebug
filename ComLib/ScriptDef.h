#pragma once
#include <list>
#include <string>

using namespace std;

enum VariateType {
    em_var_base,        //eg: int,str
    em_var_buildin      //eg: $esp $eip
};

struct VariateDesc {
    VariateType mVarType;
    int mVarLength;
    int mVarName;
    string mContent;
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

    // for express
    string mExpress;

    // for function
    FunctionDesc mFunction;

    // for command
    string mCommand;
};

enum LogicNodeType {
    em_logic_order,
    em_logic_if,
    em_logic_elseif,
    em_logic_else,
    em_logic_while
};

struct LogicNode {
    LogicNodeType mLogicType;   //logic type

    LogicNode *mNext;           //logic for logic order
    LogicNode *mLeft;           //logic for right eg:if, else if, 
    LogicNode *mRight;          //logic for wrong

    list<ExpressionNode> mExpressionSet;    //expression list

    list<VariateDesc> mVarSet;  //var set
    list<FunctionDesc> mFunSet; //function set
};