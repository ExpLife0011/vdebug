#ifndef SYNTAXVIEW_H_H_
#define SYNTAXVIEW_H_H_
#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include "ViewBase.h"
#include "SyntaxCfg.h"

using namespace std;

struct SynbaxViewAttr
{
    COLORREF m_dwViewColour;
    COLORREF m_dwDefTextColour;
    COLORREF m_dwDefBackColour;
    COLORREF m_dwSelectLineColour;
    BOOL m_bLineNum;
    HFONT m_hDefaultFont;

    SynbaxViewAttr()
    {
        m_dwViewColour = RGB(255, 0, 0);
        m_dwDefBackColour = RGB(125, 125, 125);
        m_dwDefTextColour = RGB(0, 0, 255);
        m_dwSelectLineColour = RGB(155, 155, 155);
        m_bLineNum = FALSE;
        m_hDefaultFont = NULL;
    }
};

struct SyntaxColourNode
{
    std::wstring m_wstrContent;
    DWORD m_dwStartPos;
    DWORD m_dwLength;

    SyntaxColourDesc m_vHightLightDesc;

    SyntaxColourNode(
        const ustring &wstrKeyWord,
        DWORD dwStartPos = 0,
        SyntaxColourDesc vDesc = SyntaxColourDesc()
        )
    {
        m_wstrContent = wstrKeyWord;
        m_dwStartPos = dwStartPos;
        m_dwLength = wstrKeyWord.size();
        m_vHightLightDesc = vDesc;
    }
};

struct SyntaxDesc
{
    vector<vector<SyntaxColourNode>> m_vSyntaxDesc;
    vector<ustring> m_vShowInfo;

    bool operator+=(const SyntaxDesc &desc)
    {
        m_vSyntaxDesc.insert(m_vSyntaxDesc.end(), desc.m_vSyntaxDesc.begin(), desc.m_vSyntaxDesc.end());
        m_vShowInfo.insert(m_vShowInfo.end(), desc.m_vShowInfo.begin(), desc.m_vShowInfo.end());
        return true;
    }

    bool operator=(const SyntaxDesc &desc)
    {
        m_vSyntaxDesc = desc.m_vSyntaxDesc;
        m_vShowInfo = desc.m_vShowInfo;
        return true;
    }

    BOOL IsValid() const
    {
        return TRUE;
    }

    VOID Clear()
    {
        m_vSyntaxDesc.clear();
        m_vShowInfo.clear();
    }
};


class CSynbaxView : public CWindowBase
{
public:
    CSynbaxView()
    {
        m_dwLineOfPage = 0;
        m_dwCurPos = 0;
        m_dwMaxPos = 0;
        m_bYScrollShow = FALSE;
        m_dwLineHight = 0;
        m_dwSelectLine = 0xffffffff;

        m_hMemDc = NULL;
        m_hMemBmp = NULL;
        m_hOldBmp = NULL;
    }

    virtual ~CSynbaxView()
    {}

    bool SetSynbaxViewAttr(const SynbaxViewAttr &vAttr);
    BOOL CreateSynbaxView(HWND hParent, DWORD dwWidth, DWORD dwHight);
    void AppendSyntaxDesc(const SyntaxDesc &vDesc);
    void Redraw();
    bool ReloadSynbaxCfg(LPCWSTR wszCfgJson);
    void ClearView();
    virtual BOOL SetFont(HFONT hFont);

protected:
    BOOL RegisterSynbaxClass();
    void OnPaintStr(HDC hdc, DWORD dwX, DWORD dwY) const;
    void LoadGlobalCfg(const Value &vGlobal);

protected:
    LRESULT OnCreate(WPARAM wp, LPARAM lp);
    LRESULT OnLButtonDown(WPARAM wp, LPARAM lp);
    LRESULT OnMouseMove(WPARAM wp, LPARAM lp);
    LRESULT OnLButtonUp(WPARAM wp, LPARAM lp);
    LRESULT OnPaint(WPARAM wp, LPARAM lp) const;
    LRESULT OnClose(WPARAM wp, LPARAM lp);

protected:
    SynbaxViewAttr m_vAttr;
    vector<ustring> m_vShowInfo;
    vector<vector<SyntaxColourNode>> m_vSyntaxRules;

protected:
    virtual LRESULT OnWindowMsg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void OnSize(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnVscroll(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnAppendDesc(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnClearView(HWND hwnd, WPARAM wp, LPARAM lp);
    void SetVscrollPos(DWORD dwPos);
    void ReCalParam();

protected:
    HDC m_hMemDc;
    HBITMAP m_hMemBmp;
    HBITMAP m_hOldBmp;

    //滚动条控制相关数据
    DWORD m_dwLineHight;        //行高
    DWORD m_dwLineOfPage;       //一页包含的行数
    DWORD m_dwCurPos;           //当前所在的位置（行数）
    DWORD m_dwMaxPos;           //Y轴最大的滑动范围
    BOOL m_bYScrollShow;        //是否展示纵向滚动条
    DWORD m_dwSelectLine;       //当前选择的行，高亮处理
};
#endif