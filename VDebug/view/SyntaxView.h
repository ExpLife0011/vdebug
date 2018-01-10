#ifndef SYNTAXVIEW_H_H_
#define SYNTAXVIEW_H_H_
#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include "ViewBase.h"
#include "SyntaxCfg.h"
#include "SyntaxDesc.h"

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
        m_dwSelectStart = 0;
        m_dwSelectEnd = 0;
        m_bLButtonDown = FALSE;
    }

    virtual ~CSynbaxView()
    {}

    bool SetSynbaxViewAttr(const SynbaxViewAttr &vAttr);
    BOOL CreateSynbaxView(HWND hParent, DWORD dwWidth, DWORD dwHight);
    void AppendSyntaxDesc(const SyntaxDesc &vDesc);
    void AppendSyntaxDescA(const mstring &strTest);
    void Redraw();
    bool ReloadSynbaxCfg(LPCWSTR wszCfgJson);
    void ClearView();
    virtual BOOL SetFont(HFONT hFont);

protected:
    BOOL RegisterSynbaxClass();
    void OnPaintStr(HDC hdc, DWORD dwX, DWORD dwY) const;
    void LoadGlobalCfg(const Value &vGlobal);

protected:
    DWORD GetLineNumFromCoord(short iX, short iY);
    LRESULT OnCreate(WPARAM wp, LPARAM lp);
    LRESULT OnLButtonDown(WPARAM wp, LPARAM lp);
    LRESULT OnMouseMove(WPARAM wp, LPARAM lp);
    LRESULT OnLButtonUp(WPARAM wp, LPARAM lp);
    LRESULT OnRButtonUp(WPARAM wp, LPARAM lp);
    LRESULT OnPaint(WPARAM wp, LPARAM lp) const;
    LRESULT OnClose(WPARAM wp, LPARAM lp);

protected:
    SynbaxViewAttr m_vAttr;
    vector<mstring> m_vShowInfo;
    vector<vector<SyntaxColourNode>> m_vSyntaxRules;

protected:
    virtual LRESULT OnWindowMsg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void OnSize(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnVscroll(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnAppendDesc(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnAppendMsg(HWND hwnd, WPARAM wp, LPARAM lp);
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

    BOOL m_bLButtonDown;        //左键是否按下
    DWORD m_dwSelectBase;       //鼠标按下的初始位置
    DWORD m_dwSelectStart;      //当前选中的起始行
    DWORD m_dwSelectEnd;        //当前选中的结束行
};
#endif