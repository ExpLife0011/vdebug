#ifndef SYNTAXPARSER_H_H_
#define SYNTAXPARSER_H_H_
#include <Windows.h>
#include <string>
#include <list>
#include <SyntaxView/export.h>

#define SCI_LABEL_DEFAULT       "Default"
#define SCI_LABEL_CALLSTACK     "CallStack"

#define SCI_PARSER_STAT_DEFAULT     1       //for default style
#define SCI_PARSER_STAT_NUMBER      2       //for number style
#define SCI_PARSER_STAT_FUNCTION    3       //for function msg
#define SCI_PARSER_STAT_ADDR        4
#define SCI_PARSER_STAT_REGISTER    5
#define SCI_PARSER_STAT_ERROR       6
#define SCI_PARSER_STAT_MESSAGE     7
#define SCI_PARSER_STAT_HEX         8
#define SCI_PARSER_STAT_DATA        9
#define SCI_PARSER_STAT_BYTE        10
#define SCI_PARSER_STAT_INST        11
#define SCI_PARSER_STAT_CALL        12
#define SCI_PARSER_STAT_JMP         13
#define SCI_PARSER_STAT_PROC        14
#define SCI_PARSER_STAT_MODULE      15
#define SCI_PARSER_STAT_PARAM       16
#define SCI_PARSER_STAT_KEYWORD     17
#define SCI_PARSER_STAT_HIGHT       18

typedef void (__stdcall *pfnRegisterSyntaxProc)(const char *label, pfnColouriseTextProc pfn);

struct SyntaxParserMsg {
    int m_stat;
    int m_startPos;
    int m_endPos;
    int m_length;
    std::string m_content;
};

class SyntaxParser {
private:
    SyntaxParser();
    virtual ~SyntaxParser();

public:
    static SyntaxParser *GetInstance();
    bool InitParser();

private:
    static bool IsValidChar(char c);
    static std::list<SyntaxParserMsg> CallStackParserForLine(unsigned int startPos, const std::string &content);
    bool RegisterSyntaxProc(const std::string &label, pfnColouriseTextProc pfn);
    static std::list<SyntaxParserMsg> SplitByLine(int startPos, int length, const std::string &content);
    static void __stdcall CallStackParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc
        );
    static void __stdcall DefaultParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc
        );

private:
    pfnRegisterSyntaxProc m_pfnRegister;
};
#endif //SYNTAXPARSER_H_H_