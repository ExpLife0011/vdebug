#include "FunSyntaxView.h"

CFunSyntaxView::CFunSyntaxView() {
}

CFunSyntaxView::~CFunSyntaxView() {
}

bool CFunSyntaxView::InitFunView() {
    ShowMargin(false);
    SetCaretColour(RGB(255, 255, 255));

    SetFont("Lucida Console");
    SetCaretSize(1);

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);
    return true;
}