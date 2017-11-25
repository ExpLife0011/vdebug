#ifndef SYNTAXCFG_H_H_
#define SYNTAXCFG_H_H_
#include <Windows.h>
#include "../global.h"

/*语法高亮配置 开始*/
#define COLOUR_ADDR             GetSyntaxCfg(L"addr")           //地址
#define COLOUR_REGISTER         GetSyntaxCfg(L"register")       //寄存器
#define COLOUR_ERROR            GetSyntaxCfg(L"error")          //错误
#define COLOUR_MSG              GetSyntaxCfg(L"msg")            //普通信息
#define COLOUR_HEX              GetSyntaxCfg(L"hex")            //哈希值
#define COLOUR_DATA             GetSyntaxCfg(L"data")           //数据
#define COLOUR_BYTE             GetSyntaxCfg(L"btye")           //字节码
#define COLOUR_INST             GetSyntaxCfg(L"inst")           //指令集
#define COLOUR_CALL             GetSyntaxCfg(L"call")           //call
#define COLOUR_JMP              GetSyntaxCfg(L"jmp")            //jmp
#define COLOUR_PROC             GetSyntaxCfg(L"proc")           //函数名
#define COLOUR_MODULE           GetSyntaxCfg(L"module")         //模块名
#define COLOUR_PARAM            GetSyntaxCfg(L"param")          //参数
#define COLOUR_KEYWORD          GetSyntaxCfg(L"keyword")        //关键字
#define COLOUR_NUM              GetSyntaxCfg(L"num")            //数字
/*语法高亮配置 结束*/

struct SyntaxColourDesc
{
    DWORD m_dwTextColour;
    DWORD m_dwBackColour;

    BOOL m_bBold;
    BOOL m_bItalic;

    SyntaxColourDesc(
        DWORD dwTextColour = NULL_COLOUR,
        DWORD dwBackColour = NULL_COLOUR,
        BOOL bBold = FALSE,
        BOOL bItalic = FALSE
        )
    {
        m_dwTextColour = dwTextColour;
        m_dwBackColour = dwBackColour;
        m_bBold = bBold;
        m_bItalic = bItalic;
    }

    bool IsValid()
    {
        return (m_dwBackColour || m_dwTextColour);
    }
};

BOOL LoadSyntaxCfg(const Value &vSyntaxCfg);

SyntaxColourDesc GetSyntaxCfg(LPCWSTR wszName);
#endif