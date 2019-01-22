#ifndef SYNTAXCFG_H_H_
#define SYNTAXCFG_H_H_
#include <Windows.h>
#include <string>
#include <ComStatic/ComStatic.h>
#include "SyntaxParser.h"
#include "SyntaxView.h"

#define NULL_COLOUR 0xffffffff

/*语法高亮配置 开始*/
#define COLOUR_DEFAULT          GetSyntaxCfg(SCI_PARSER_STAT_DEFAULT)       //默认配置
#define COLOUR_ADDR             GetSyntaxCfg(SCI_PARSER_STAT_ADDR)          //地址
#define COLOUR_REGISTER         GetSyntaxCfg(SCI_PARSER_STAT_REGISTER)      //寄存器
#define COLOUR_ERROR            GetSyntaxCfg(SCI_PARSER_STAT_ERROR)         //错误
#define COLOUR_MSG              GetSyntaxCfg(SCI_PARSER_STAT_MESSAGE)       //普通信息
#define COLOUR_HEX              GetSyntaxCfg(SCI_PARSER_STAT_HEX)           //哈希值
#define COLOUR_DATA             GetSyntaxCfg(SCI_PARSER_STAT_DATA)          //数据
#define COLOUR_BYTE             GetSyntaxCfg(SCI_PARSER_STAT_BYTE)          //字节码
#define COLOUR_INST             GetSyntaxCfg(SCI_PARSER_STAT_INST)          //指令集
#define COLOUR_CALL             GetSyntaxCfg(SCI_PARSER_STAT_CALL)          //call
#define COLOUR_JMP              GetSyntaxCfg(SCI_PARSER_STAT_JMP)           //jmp
#define COLOUR_PROC             GetSyntaxCfg(SCI_PARSER_STAT_PROC)          //函数名
#define COLOUR_MODULE           GetSyntaxCfg(SCI_PARSER_STAT_MODULE)        //模块名
#define COLOUR_PARAM            GetSyntaxCfg(SCI_PARSER_STAT_PARAM)         //参数
#define COLOUR_KEYWORD          GetSyntaxCfg(SCI_PARSER_STAT_KEYWORD)       //关键字
#define COLOUR_NUM              GetSyntaxCfg(SCI_PARSER_STAT_NUMBER)        //数字
#define COLOUR_HIGHT            GetSyntaxCfg(SCI_PARSER_STAT_HIGHT)         //高亮
/*语法高亮配置 结束*/

struct SyntaxColourDesc
{
    std::mstring m_strDesc;
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

BOOL UpdateSyntaxView(SyntaxView *pSyntaxView);

BOOL LoadSyntaxCfg(const std::string &path);

BOOL ReloadSyntaxCfg();

SyntaxColourDesc *GetSyntaxCfg(int type);
#endif