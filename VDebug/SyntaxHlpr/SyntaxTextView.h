#ifndef SYNTAXSHELL_H_H_
#define SYNTAXSHELL_H_H_
#include <Windows.h>
#include <string>
#include <set>
#include <map>
#include "include/SciLexer.h"
#include "include/Scintilla.h"
#include "SyntaxDef.h"
#include "export.h"

typedef int (* SCINTILLA_FUNC) (void*, int, int, int);
typedef void * SCINTILLA_PTR;
typedef void (__stdcall *pfnLabelParser)(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    );

#define SCLEX_LABEL 161
//LabelParser消息
//为SyntaxTextView注册解析器
//wparem : LabelParser
//lparam : no used
/*
struct LabelParser {
    const char *mLabel;
    void *mParam;
    void *mPfnParser;
};
*/
#define MSG_LABEL_REGISTER_PARSER     5051
//设置文本标签
#define MSG_LABEL_CLEAR_LABEL         5060
//追加文本标签
#define MSG_LABEL_APPEND_LABEL        5061
//设置高亮字符串
//wparam : const char *
//lparam : no used
#define MSG_SET_KEYWORK_STR           5071

typedef LRESULT (CALLBACK *PWIN_PROC)(HWND, UINT, WPARAM, LPARAM);

class SyntaxTextView {
    struct ProcHookParam {
        PWIN_PROC mOldWndProc;
        std::set<SyntaxTextView *> mSyntaxSet;

        ProcHookParam() {
            mOldWndProc = NULL;
        }
    };

public:
    SyntaxTextView();
    virtual ~SyntaxTextView();

    //SyntaxTextView Create
    bool CreateView(HWND parent, int x, int y, int cx, int cy);
    //Parser Register
    bool RegisterParser(const std::string &label, pfnLabelParser parser, void *param);
    //Send Window Message
    size_t SendMsg(UINT msg, WPARAM wp, LPARAM lp) const;
    //AppendText To View
    void AppendText(const std::string &label, const std::string &text);
    //Set View Text
    void SetText(const std::string &label, const std::string &text);
    //Get Current View Text
    std::string GetText() const;
    //Set Line Num
    void SetLineNum(bool lineNum);
    void ClearView();
    HWND GetWindow() {
        return m_hwnd;
    }

    //update view
    void UpdateView() const;

    //Auto to EndLine
    bool SetAutoScroll(bool flag);
    //Sel Jump To Next Str
    bool JmpNextPos(const std::string &str);
    bool JmpFrontPos(const std::string &str);
    bool JmpFirstPos(const std::string &str);
    bool JmpLastPos(const std::string &str);

    //Set Keyword For HighLight
    bool AddHighLight(const std::string &keyWord, DWORD colour);
    bool ClearHighLight();

    void SetStyle(int type, unsigned int textColour, unsigned int backColour);
    void ShowCaretLine(bool show, unsigned int colour, unsigned char alpha);
    void ShowMargin(bool bShow);
    void SetDefStyle(unsigned int textColour, unsigned int backColour);
    void ShowVsScrollBar(bool show);
    void ShowHsScrollBar(bool show);

    void SetFont(const std::string &fontName);
    void SetCaretColour(unsigned int colour);
    void SetCaretSize(int size);
    void SetFontWeight(int weight);
    int GetFontWeight();
    unsigned int GetCaretColour();
    int SetScrollEndLine();

protected:
    virtual void OnViewUpdate() const;

private:
    INT_PTR OnNotify(HWND hdlg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK WndSubProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void CheckLineNum();
    void ResetLineNum();
    static void Lock();
    static void UnLock();

private:
    PWIN_PROC mParentProc;
    static CRITICAL_SECTION *msLocker;
    static std::map<HWND, ProcHookParam> msWinProcCache;

    bool mLineNum;
    int mLineCount;
    bool mAutoScroll;

    HWND m_hwnd;
    HWND m_parent;
    SCINTILLA_FUNC m_pfnSend;
    SCINTILLA_PTR m_param;
    std::map<std::string, DWORD> mHighLight;
    std::string mStrInView;
};
#endif //SYNTAXSHELL_H_H_