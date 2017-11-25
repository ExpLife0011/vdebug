#ifndef SYNTAXCFG_H_H_
#define SYNTAXCFG_H_H_
#include <Windows.h>
#include "../global.h"

/*�﷨�������� ��ʼ*/
#define COLOUR_ADDR             GetSyntaxCfg(L"addr")           //��ַ
#define COLOUR_REGISTER         GetSyntaxCfg(L"register")       //�Ĵ���
#define COLOUR_ERROR            GetSyntaxCfg(L"error")          //����
#define COLOUR_MSG              GetSyntaxCfg(L"msg")            //��ͨ��Ϣ
#define COLOUR_HEX              GetSyntaxCfg(L"hex")            //��ϣֵ
#define COLOUR_DATA             GetSyntaxCfg(L"data")           //����
#define COLOUR_BYTE             GetSyntaxCfg(L"btye")           //�ֽ���
#define COLOUR_INST             GetSyntaxCfg(L"inst")           //ָ�
#define COLOUR_CALL             GetSyntaxCfg(L"call")           //call
#define COLOUR_JMP              GetSyntaxCfg(L"jmp")            //jmp
#define COLOUR_PROC             GetSyntaxCfg(L"proc")           //������
#define COLOUR_MODULE           GetSyntaxCfg(L"module")         //ģ����
#define COLOUR_PARAM            GetSyntaxCfg(L"param")          //����
#define COLOUR_KEYWORD          GetSyntaxCfg(L"keyword")        //�ؼ���
#define COLOUR_NUM              GetSyntaxCfg(L"num")            //����
/*�﷨�������� ����*/

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