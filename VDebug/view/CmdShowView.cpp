#include "CmdShowView.h"
#include "../SyntaxHlpr/SyntaxDef.h"

using namespace std;

CCmdShowView::CCmdShowView() {
}

CCmdShowView::~CCmdShowView() {
}

bool CCmdShowView::InitShowView() {
    ShowMargin(false);
    SetCaretColour(RGB(255, 255, 255));

    SetFont("Lucida Console");
    SetCaretSize(1);

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);
    InitCache(50);
    SetAutoScroll(true);

    //当前选中行
    ShowCaretLine(true, RGB(0xff, 0xff, 0xff), 50);

    RegisterParser(LABEL_DBG_SEND, SendDefaultParser, NULL);
    RegisterParser(LABEL_DBG_RECV, RecvDefaultParser, NULL);
    RegisterParser(LABEL_DBG_MODULE, ModuleLoadedParser, NULL);
    RegisterParser(LABEL_DBG_CALLSTACK, CallStackParser, NULL);
    return true;
}

bool CCmdShowView::LoadUserCfg(const CStyleConfig &cfg) {
    StyleConfigInfo info = cfg.GetStyleConfig();

    //加载用户配置项
    if (info.mLineNum)
    {
        ShowMargin(true);
        SetLineNum(true);
    } else {
        ShowMargin(false);
        SetLineNum(false);
    }
    SetFont(info.mFontName);
    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, info.mFontSize);

    //选择区域背景色和透明度
    SendMsg(SCI_SETSELBACK, true, info.mSelColour);
    SendMsg(SCI_SETSELALPHA, info.mSelAlpha, 0);

    map<mstring, StyleConfigNode>::const_iterator it;
    for (it = info.mCfgSet.begin() ; it != info.mCfgSet.end() ; it++)
    {
        const StyleConfigNode &cur = it->second;
        if (it->first == "default")
        {
            SetDefStyle(cur.mRgbText, cur.mRgbBack);
        } else {
            SetStyle(cur.mSyntaxStyle, cur.mRgbText, cur.mRgbBack);
        }
    }
    UpdateView();
    return true;
}

void CCmdShowView::SendDefaultParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    s->SetState(STYLE_DBG_SEND_DEFAULT);
    s->ForwardBytes(length);
}

void CCmdShowView::RecvDefaultParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    s->SetState(STYLE_DBG_RECV_DEFAULT);
    s->ForwardBytes(length);
}

void CCmdShowView::ModuleLoadedParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    s->SetState(STYLE_DBG_RECV_DEFAULT);
    s->ForwardBytes(length);
}

void CCmdShowView::CallStackParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    s->SetState(STYLE_DBG_RECV_DEFAULT);
    s->ForwardBytes(length);
}