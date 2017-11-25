#include <Windows.h>
#include <sstream>
#include <fstream>
#include "SyntaxView.h"
#include "common.h"

#define MSG_APPEND_DESC   (WM_USER + 5060)
#define MSG_CLEAR_VIEW    (WM_USER + 5061)

extern HINSTANCE g_hInstance;
#define CLASS_VDEBUG_SYNTAXVIEW     L"VDebugSynbaxViewClass"

LRESULT CSynbaxView::OnWindowMsg(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    LRESULT pResult = 0;
    switch (msg)
    {
    case  WM_CREATE:
        OnCreate(wp, lp);
        break;
    case  WM_PAINT:
        OnPaint(wp, lp);
        break;
    case WM_ERASEBKGND:
        pResult = 1;
        break;
    case WM_SIZE:
        OnSize(hwnd, wp, lp);
        break;
    case  WM_LBUTTONDOWN:
        {
            OnLButtonDown(wp, lp);
        }
        break;
    case  WM_MOUSEMOVE:
        break;
    case  WM_LBUTTONUP:
        break;
    case WM_MOUSEWHEEL:
        OnMouseWheel(hwnd, wp, lp);
        break;
    case  WM_VSCROLL:
        OnVscroll(hwnd, wp, lp);
        break;
    case MSG_APPEND_DESC:
        OnAppendDesc(hwnd, wp, lp);
        break;
    case MSG_CLEAR_VIEW:
        OnClearView(hwnd, wp, lp);
        break;
    case  WM_CLOSE:
        OnClose(wp, lp);
        break;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return pResult;
}

BOOL CSynbaxView::RegisterSynbaxClass()
{
    WNDCLASSW vWndClass = {0};
    vWndClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    vWndClass.lpfnWndProc = NULL;
    vWndClass.hInstance = g_hInstance;
    vWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    vWndClass.lpszClassName = CLASS_VDEBUG_SYNTAXVIEW;
    return RegisterWnd(vWndClass);
}

BOOL CSynbaxView::CreateSynbaxView(HWND hParent, DWORD dwWidth, DWORD dwHight)
{
    if (!RegisterSynbaxClass())
    {
        return false;
    }

    return CreateWnd(hParent, 0, 0, dwWidth, dwHight, WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL);
}

void CSynbaxView::AppendSyntaxDesc(const SyntaxDesc &vDesc)
{
    if (!vDesc.IsValid())
    {
        return;
    }

    //线程同步
    SyntaxDesc *ptr = new SyntaxDesc;
    *ptr = vDesc;
    PostMessageW(m_hwnd, MSG_APPEND_DESC, (WPARAM)ptr, 0);
}

bool CSynbaxView::ReloadSynbaxCfg(LPCWSTR wszCfgJson)
{
    if (!wszCfgJson || !wszCfgJson[0])
    {
        return false;
    }

    try
    {
        fstream fp(wszCfgJson);
        if (!fp.is_open())
        {
            return false;
        }

        Reader reader;
        Value vContent;
        if (!reader.parse(fp, vContent) || objectValue != vContent.type())
        {
            return false;
        }

        vector<string> vCfgRoot = vContent.getMemberNames();
        vector<string>::const_iterator itSyntaxCfg;
        for (itSyntaxCfg = vCfgRoot.begin() ; itSyntaxCfg != vCfgRoot.end() ; itSyntaxCfg++)
        {
            if (*itSyntaxCfg == "globalCfg")
            {
                LoadGlobalCfg(vContent["globalCfg"]);
                continue;
            }

            if (*itSyntaxCfg == "SyntaxCfg")
            {
                LoadSyntaxCfg(vContent["SyntaxCfg"]);
            }
        }

        Redraw();
    }
    catch (exception &e)
    {
        dp(L"Err:%hs", e.what());
    }

    return true;
}

void CSynbaxView::SetVscrollPos(DWORD dwPos)
{
    if (dwPos > (m_dwMaxPos - m_dwLineOfPage + 1))
    {
        dwPos = (m_dwMaxPos - m_dwLineOfPage + 1);
    }
    m_dwCurPos = dwPos;
    SetScrollPos(m_hwnd, SB_VERT, m_dwCurPos, TRUE);
    InvalidateRect(m_hwnd, NULL, TRUE);
}

void CSynbaxView::OnAppendDesc(HWND hwnd, WPARAM wp, LPARAM lp)
{
    SyntaxDesc *ptr = (SyntaxDesc *)wp;

    if (m_vSyntaxRules.size() != m_vShowInfo.size())
    {
        ErrMessage(L"数据数量错误");
        return;
    }
    m_vSyntaxRules.insert(m_vSyntaxRules.end(), ptr->m_vSyntaxDesc.begin(), ptr->m_vSyntaxDesc.end());
    m_vShowInfo.insert(m_vShowInfo.end(), ptr->m_vShowInfo.begin(), ptr->m_vShowInfo.end());
    ReCalParam();

    if (m_bYScrollShow)
    {
        SetVscrollPos(m_dwMaxPos);
    }
    InvalidateRect(m_hwnd, NULL, TRUE);
    delete ptr;
}

void CSynbaxView::OnClearView(HWND hwnd, WPARAM wp, LPARAM lp)
{
    m_vSyntaxRules.clear();
    m_vShowInfo.clear();
    ReCalParam();
    InvalidateRect(m_hwnd, NULL, TRUE);
}

void CSynbaxView::OnMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp)
{
    if (!m_bYScrollShow)
    {
        return;
    }

    int iMove = (-((short)HIWORD(wp) / WHEEL_DELTA));
    if (iMove > 0)
    {
        SetVscrollPos(m_dwCurPos + iMove);
    }
    else
    {
        int iPos = (int)m_dwCurPos;
        iPos += iMove;
        if (iPos < 0)
        {
            iPos = 0;
        }
        SetVscrollPos(iPos);
    }
}

void CSynbaxView::OnSize(HWND hwnd, WPARAM wp, LPARAM lp)
{
    ReCalParam();
    if (m_bYScrollShow)
    {
        SetVscrollPos(m_dwMaxPos);
    }
}

void CSynbaxView::OnVscroll(HWND hwnd, WPARAM wp, LPARAM lp)
{
    DWORD dwCode = LOWORD(wp);
    switch (dwCode)
    {
    case SB_LINEDOWN:
        {
            dp(L"SB_LINEDOWN");
        }
        break;
    case SB_LINEUP:
        {
            dp(L"SB_LINEUP");
        }
        break;
    case SB_PAGEDOWN:
        {
            dp(L"SB_PAGEDOWN");
        }
        break;
    case SB_PAGEUP:
        {
            dp(L"SB_PAGEUP");
        }
        break;
    case  SB_THUMBTRACK:
        {
            dp(L"SB_THUMBTRACK");
            SCROLLINFO scr = {0};
            scr.cbSize = sizeof(scr);
            scr.fMask = SIF_TRACKPOS;
            if (GetScrollInfo(m_hwnd, SB_VERT, &scr))
            {
                dp(L"nTrackPos:%d, max:%d, c:%d", scr.nTrackPos, m_dwMaxPos, m_dwLineOfPage);
                SetVscrollPos(scr.nTrackPos);
            }
        }
        break;
    }
}

void CSynbaxView::Redraw()
{
    //m_vSyntaxRules.clear();
    //vector<SyntaxShowInfo>::const_iterator it;
    //for (it = m_vShowInfo.begin() ; it != m_vShowInfo.end() ; it++)
    //{
    //    if (m_vParser.end() != m_vParser.find(it->m_wstrClass))
    //    {
    //        const CParserBase *ptr = m_vParser[it->m_wstrClass];
    //        m_vSyntaxRules.push_back(ptr->ParserString(it->m_wstrLine.c_str()));
    //    }
    //}
    InvalidateRect(m_hwnd, NULL, TRUE);
}

void CSynbaxView::ClearView()
{
    PostMessageW(m_hwnd, MSG_CLEAR_VIEW, 0, 0);
}

void CSynbaxView::LoadGlobalCfg(const Value &vGlobal)
{
    try
    {
        m_vAttr.m_bLineNum = JsonGetIntValue(vGlobal, "lineNumber");
        m_vAttr.m_dwDefTextColour = GetColourFromStr(JsonGetStrValue(vGlobal, "defTextColour").c_str());
        m_vAttr.m_dwDefBackColour = GetColourFromStr(JsonGetStrValue(vGlobal, "defBackColour").c_str());
        m_vAttr.m_dwViewColour = GetColourFromStr(JsonGetStrValue(vGlobal, "viewBackColour").c_str());
        m_vAttr.m_dwSelectLineColour = GetColourFromStr(JsonGetStrValue(vGlobal, "curLineColour").c_str());
    }
    catch (exception &e)
    {
        dp(L"Err:%hs", e.what());
    }
}

BOOL CSynbaxView::SetFont(HFONT hFont)
{
    m_vAttr.m_hDefaultFont = hFont;
    Redraw();
    return TRUE;
}

//重新计算展示参数
void CSynbaxView::ReCalParam()
{
    HDC hdc = GetWindowDC(m_hwnd);
    SIZE sz = {0};
    HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)m_vAttr.m_hDefaultFont);
    GetTextExtentPoint32W(hdc, L"0", 1, &sz);
    SelectObject(hdc, hOldFont);
    m_dwLineHight = sz.cy;

    RECT rtClient = {0};
    GetClientRect(m_hwnd, &rtClient);

    m_dwLineOfPage = (rtClient.bottom - rtClient.top) / sz.cy;

    SCROLLINFO scr = {0};
    scr.fMask = SIF_ALL;
    scr.cbSize = sizeof(scr);
    if (m_dwLineOfPage < m_vSyntaxRules.size())
    {
        m_bYScrollShow = TRUE;
        ShowScrollBar(m_hwnd, SB_VERT, TRUE);
        m_dwMaxPos = (m_vSyntaxRules.size());
        scr.nPage = m_dwLineOfPage;
        scr.nMax = m_dwMaxPos;
        if (m_dwCurPos > m_dwMaxPos)
        {
            m_dwCurPos = m_dwMaxPos;
        }
        scr.nPos = m_dwCurPos;
        SetScrollInfo(m_hwnd, SB_VERT, &scr, TRUE);
    }
    else
    {
        m_bYScrollShow = FALSE;
        ShowScrollBar(m_hwnd, SB_VERT, FALSE);
        m_dwCurPos = 0;
        m_dwMaxPos = 0;
    }
    ReleaseDC(m_hwnd, hdc);
    InvalidateRect(m_hwnd, NULL, TRUE);
}

static INT CALLBACK _EnumFontNameProc(LOGFONTW *plf, TEXTMETRICW* /*ptm*/, INT /*nFontType*/, LPARAM lParam)
{
    dp(L"font:%ls", plf->lfFaceName);
    HFONT *ptr = (HFONT *)lParam;
    
    if (0 == lstrcmpiW(plf->lfFaceName, L"Lucida Console"))
    {
        LOGFONTW ft = {0};
        memcpy(&ft, plf, sizeof(LOGFONTW));
        ft.lfHeight = 0;
        ft.lfWidth = 0;
        (*ptr) = CreateFontIndirectW(&ft);
    }
    return TRUE;
}

LRESULT CSynbaxView::OnCreate(WPARAM wp, LPARAM lp)
{
    ShowScrollBar(m_hwnd, SB_VERT, FALSE);

    HWND hDesktop = GetDesktopWindow();
    HDC hScreenDC = GetWindowDC(hDesktop);
    DWORD dwCX = GetDeviceCaps(hScreenDC, HORZRES);
    DWORD dwCY = GetDeviceCaps(hScreenDC, VERTRES);
    ReleaseDC(hDesktop, hScreenDC);

    HDC hWndDc = GetWindowDC(m_hwnd);
    m_hMemDc = CreateCompatibleDC(hWndDc);
    m_hMemBmp = CreateCompatibleBitmap(
        hWndDc,
        dwCX,
        dwCY
        );
    m_hOldBmp = (HBITMAP)SelectObject(m_hMemDc, (HGDIOBJ)m_hMemBmp);
    ReleaseDC(m_hwnd, hWndDc);
    return 0;
}

LRESULT CSynbaxView::OnLButtonDown(WPARAM wp, LPARAM lp)
{
    DWORD dwX = LOWORD(lp);
    DWORD dwY = HIWORD(lp);

    HDC dc = GetWindowDC(m_hwnd);
    SIZE sz = {0};
    GetTextExtentPoint32W(dc, L"0", 1, &sz);
    ReleaseDC(m_hwnd, dc);

    if (dwY > (DWORD)(sz.cy / 2))
    {
        dwY += 3;
    }
    //根据坐标获取所在行数
    DWORD dwLineInCurPage = ((dwY / m_dwLineHight));
    m_dwSelectLine = (m_dwCurPos + dwLineInCurPage);
    if (m_dwSelectLine < m_vSyntaxRules.size())
    {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
    return 0;
}

LRESULT CSynbaxView::OnMouseMove(WPARAM wp, LPARAM lp)
{
    return 0;
}

LRESULT CSynbaxView::OnLButtonUp(WPARAM wp, LPARAM lp)
{
    return 0;
}

void CSynbaxView::OnPaintStr(HDC hdc, DWORD dwX, DWORD dwY) const
{
    vector<vector<SyntaxColourNode>>::const_iterator itSingleLineInfo;
    vector<SyntaxColourNode>::const_iterator itColourNode;
    DWORD dwLine = 0;
    RECT rtText = {0};
    rtText.left = dwX;
    rtText.top = dwY;
    HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)m_vAttr.m_hDefaultFont);
    RECT rtClient = {0};
    GetClientRect(m_hwnd, &rtClient);
    //从展示的第一行起开始绘制
    for (itSingleLineInfo = m_vSyntaxRules.begin() + m_dwCurPos; itSingleLineInfo != m_vSyntaxRules.end() && dwLine < m_dwLineOfPage; itSingleLineInfo++, dwLine++)
    {
        SIZE szLine = {0};
        wstring wstrLine = m_vShowInfo[m_dwCurPos + dwLine];
        DWORD dwPos = 0;

        //可能只有空行
        if (wstrLine.empty())
        {
            GetTextExtentPoint32W(hdc, L" ", 1, &szLine);
        }
        else
        {
            GetTextExtentPoint32W(hdc, wstrLine.c_str(), wstrLine.size(), &szLine);
            rtText.right = rtText.left + szLine.cx;
            rtText.bottom = rtText.top + szLine.cy;
        }

        if (m_dwSelectLine == (m_dwCurPos + dwLine))
        {
            if (m_vAttr.m_dwSelectLineColour != NULL_COLOUR)
            {
                RECT rtSelect = {rtClient.left, rtText.top, rtClient.right - rtClient.left, rtText.top + szLine.cy};
                HBRUSH hBrush = CreateSolidBrush(m_vAttr.m_dwSelectLineColour);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                FillRect(hdc, &rtSelect, hBrush);
                SelectObject(hdc, hOldBrush);
                DeleteObject(hBrush);
            }
        }

        if (wstrLine.empty())
        {
            rtText.left = dwX;
            rtText.top += szLine.cy;
            continue;
        }

        for (itColourNode = itSingleLineInfo->begin() ; itColourNode != itSingleLineInfo->end() ; itColourNode++)
        {
            if (itColourNode->m_dwStartPos > dwPos)
            {
                wstring wstrJmp = wstrLine.substr(dwPos, itColourNode->m_dwStartPos - dwPos);
                SetTextColor(hdc, m_vAttr.m_dwDefTextColour);

                if (NULL_COLOUR == m_vAttr.m_dwDefBackColour)
                {
                    SetBkMode(hdc, TRANSPARENT);
                }
                else
                {
                    SetBkMode(hdc, OPAQUE);
                    SetBkColor(hdc, m_vAttr.m_dwDefBackColour);
                }
                TextOutW(hdc, rtText.left, rtText.top, wstrJmp.c_str(), wstrJmp.size());
                SIZE szJmp = {0};
                GetTextExtentPoint32W(hdc, wstrJmp.c_str(), wstrJmp.size(), &szJmp);
                rtText.left += szJmp.cx;
            }

            SIZE szNode = {0};
            GetTextExtentPoint32W(hdc, itColourNode->m_wstrContent.c_str(), itColourNode->m_wstrContent.size(), &szNode);

            if (NULL_COLOUR == itColourNode->m_vHightLightDesc.m_dwTextColour)
            {
                SetTextColor(hdc, m_vAttr.m_dwDefTextColour);
            }
            else
            {
                SetTextColor(hdc, itColourNode->m_vHightLightDesc.m_dwTextColour);
            }

            if (NULL_COLOUR == itColourNode->m_vHightLightDesc.m_dwBackColour)
            {
                SetBkMode(hdc, TRANSPARENT);
            }
            else
            {
                SetBkMode(hdc, OPAQUE);
                SetBkColor(hdc, itColourNode->m_vHightLightDesc.m_dwBackColour);
            }
            TextOutW(hdc, rtText.left, rtText.top, itColourNode->m_wstrContent.c_str(), itColourNode->m_wstrContent.size());
            rtText.left += szNode.cx;
            dwPos = (itColourNode->m_dwStartPos + itColourNode->m_dwLength);
        }

        //绘制遗漏的结束字符
        if (dwPos < wstrLine.size())
        {
            wstring wstrJmp = wstrLine.substr(dwPos, wstrLine.size() - dwPos);
            SetTextColor(hdc, m_vAttr.m_dwDefTextColour);

            if (NULL_COLOUR == m_vAttr.m_dwDefBackColour)
            {
                SetBkMode(hdc, TRANSPARENT);
            }
            else
            {
                SetBkMode(hdc, OPAQUE);
                SetBkColor(hdc, m_vAttr.m_dwDefBackColour);
            }
            TextOutW(hdc, rtText.left, rtText.top, wstrJmp.c_str(), wstrJmp.size());
            SIZE szJmp = {0};
            GetTextExtentPoint32W(hdc, wstrJmp.c_str(), wstrJmp.size(), &szJmp);
        }
        rtText.left = dwX;
        rtText.top += szLine.cy;
    }

    if (hOldFont)
    {
        SelectObject(hdc, (HGDIOBJ)hOldFont);
    }
}

LRESULT CSynbaxView::OnPaint(WPARAM wp, LPARAM lp) const
{
    PAINTSTRUCT ps = {0};
    HDC hdc = BeginPaint(m_hwnd, &ps);
    RECT client = {0};
    GetClientRect(m_hwnd, &client);

    //绘制背景色
    HBRUSH hBrush = NULL;
    HBRUSH hOldBrush = NULL;
    if (m_vAttr.m_dwViewColour == NULL_COLOUR)
    {
        hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    }
    else
    {
        hBrush = CreateSolidBrush(m_vAttr.m_dwViewColour);
    }
    hOldBrush = (HBRUSH)SelectObject(m_hMemDc, hBrush);
    FillRect(m_hMemDc, &client, hBrush);

    OnPaintStr(m_hMemDc, 5, 0);
    BitBlt(hdc, 0, 0, client.right, client.bottom, m_hMemDc, 0, 0, SRCCOPY);

    SelectObject(m_hMemDc, hOldBrush);
    DeleteObject((HGDIOBJ)hBrush);
    ReleaseDC(m_hwnd, hdc);
    EndPaint(m_hwnd, &ps);
    return 0;
}

LRESULT CSynbaxView::OnClose(WPARAM wp, LPARAM lp)
{
    if (m_hMemDc && m_hMemBmp && m_hOldBmp)
    {
        SelectObject(m_hMemDc, m_hOldBmp);
        DeleteObject(m_hMemBmp);
        DeleteDC(m_hMemDc);
        m_hMemDc = NULL;
        m_hMemBmp = NULL;
        m_hOldBmp = NULL;
    }
    return 0;
}