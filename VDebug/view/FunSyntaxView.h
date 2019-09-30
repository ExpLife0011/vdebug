#pragma once
#include <Windows.h>
#include "../SyntaxHlpr/SyntaxTextView.h"

class CFunSyntaxView : public SyntaxTextView {
public:
    CFunSyntaxView();
    virtual ~CFunSyntaxView();

    bool InitFunView();
private:
};