#ifndef SYNTAXCFG_H_H_
#define SYNTAXCFG_H_H_
#include <Windows.h>
#include <string>
#include <ComStatic/ComStatic.h>
#include "SyntaxParser.h"
#include "SyntaxView.h"

#define NULL_COLOUR 0xffffffff

/*�﷨�������� ��ʼ*/
#define COLOUR_DEFAULT          GetSyntaxCfg(SCI_PARSER_STAT_DEFAULT)       //Ĭ������
#define COLOUR_ADDR             GetSyntaxCfg(SCI_PARSER_STAT_ADDR)          //��ַ
#define COLOUR_REGISTER         GetSyntaxCfg(SCI_PARSER_STAT_REGISTER)      //�Ĵ���
#define COLOUR_ERROR            GetSyntaxCfg(SCI_PARSER_STAT_ERROR)         //����
#define COLOUR_MSG              GetSyntaxCfg(SCI_PARSER_STAT_MESSAGE)       //��ͨ��Ϣ
#define COLOUR_HEX              GetSyntaxCfg(SCI_PARSER_STAT_HEX)           //��ϣֵ
#define COLOUR_DATA             GetSyntaxCfg(SCI_PARSER_STAT_DATA)          //����
#define COLOUR_BYTE             GetSyntaxCfg(SCI_PARSER_STAT_BYTE)          //�ֽ���
#define COLOUR_INST             GetSyntaxCfg(SCI_PARSER_STAT_INST)          //ָ�
#define COLOUR_CALL             GetSyntaxCfg(SCI_PARSER_STAT_CALL)          //call
#define COLOUR_JMP              GetSyntaxCfg(SCI_PARSER_STAT_JMP)           //jmp
#define COLOUR_PROC             GetSyntaxCfg(SCI_PARSER_STAT_PROC)          //������
#define COLOUR_MODULE           GetSyntaxCfg(SCI_PARSER_STAT_MODULE)        //ģ����
#define COLOUR_PARAM            GetSyntaxCfg(SCI_PARSER_STAT_PARAM)         //����
#define COLOUR_KEYWORD          GetSyntaxCfg(SCI_PARSER_STAT_KEYWORD)       //�ؼ���
#define COLOUR_NUM              GetSyntaxCfg(SCI_PARSER_STAT_NUMBER)        //����
#define COLOUR_HIGHT            GetSyntaxCfg(SCI_PARSER_STAT_HIGHT)         //����
/*�﷨�������� ����*/

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