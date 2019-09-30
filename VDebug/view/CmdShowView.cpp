#include "CmdShowView.h"
#include "../SyntaxHlpr/SyntaxDef.h"

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

    RegisterParser(LABEL_DBG_SEND, SendDefaultParser, NULL);
    RegisterParser(LABEL_DBG_RECV, RecvDefaultParser, NULL);
    return true;
}

bool CCmdShowView::LoadUserCfg(const CStyleConfig &cfg) {
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