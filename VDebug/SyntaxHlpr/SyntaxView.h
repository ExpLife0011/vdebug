#ifndef SYNTAXSHELL_H_H_
#define SYNTAXSHELL_H_H_
#include <Windows.h>
#include <string>
#include <map>
#include "SyntaxParser.h"
//#include <SyntaxHlpr/SyntaxCfg.h>

typedef int (* SCINTILLA_FUNC) (void*, int, int, int);
typedef void * SCINTILLA_PTR;

class SyntaxView {
public:
    SyntaxView();
    virtual ~SyntaxView();

    bool CreateView(HWND parent, int x, int y, int cx, int cy);
    size_t SendMsg(UINT msg, WPARAM wp, LPARAM lp) const;
    void AppendText(const std::string &label, const std::string &text) const;
    void SetText(const std::string &label, const std::string &text) const;
    void ClearView();
    HWND GetWindow() {
        return m_hwnd;
    }
    void SetStyle(int type, unsigned int textColour, unsigned int backColour);
    void ShowCaretLine(bool show, unsigned int colour);
    void ShowMargin(bool bShow);
    void SetDefStyle(unsigned int textColour, unsigned int backColour);
    void ShowScrollBar(bool show);
    void LoadSyntaxCfgFile(const std::string path);

private:
    std::string m_path;
    HWND m_hwnd;
    HWND m_parent;
    SCINTILLA_FUNC m_pfnSend;
    SCINTILLA_PTR m_param;
    std::map<int, std::string> m_SyntaxMap;
};
#endif //SYNTAXSHELL_H_H_