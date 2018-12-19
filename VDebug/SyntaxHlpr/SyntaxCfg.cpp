#include <Windows.h>
#include <map>
#include <string>
#include <fstream>
#include "SyntaxParser.h"
#include "json.h"
#include "mstring.h"
#include "common.h"
#include "SyntaxCfg.h"

using namespace std;
using namespace Json;

static map<mstring, int> *gs_pSyntaxMap = NULL;
static map<int, SyntaxColourDesc *> *gs_pSyntaxCfg = NULL;
static bool gs_bInit = false;

static DWORD gs_defTextColour = 0;
static DWORD gs_defBackColour = 0;
static DWORD gs_CaretLineColour = 0;

SyntaxColourDesc *GetSyntaxCfg(int type)
{
    map<int, SyntaxColourDesc *>::const_iterator it = gs_pSyntaxCfg->find(type);
    if (gs_pSyntaxCfg->end() != it)
    {
        return it->second;
    }
    return NULL;
}

static SyntaxColourDesc *_GetDescFromJson(const Value &vJson)
{
    SyntaxColourDesc *desc = new SyntaxColourDesc();
    desc->m_dwTextColour = GetColourFromStr(JsonGetStrValue(vJson, "textColour").c_str());
    desc->m_dwBackColour = GetColourFromStr(JsonGetStrValue(vJson, "backColour").c_str());
    desc->m_bBold = JsonGetIntValue(vJson, "bold");
    desc->m_bItalic = JsonGetIntValue(vJson, "italic");

    if (desc->m_dwTextColour == NULL_COLOUR)
    {
        desc->m_dwTextColour = gs_defTextColour;
    }

    if (desc->m_dwBackColour == NULL_COLOUR)
    {
        desc->m_dwBackColour = gs_defBackColour;
    }
    return desc;
}

static void _InitSyntaxCfg() {
    gs_pSyntaxMap = new map<mstring, int>();
    gs_pSyntaxCfg = new map<int, SyntaxColourDesc *>();

    gs_pSyntaxMap->insert(make_pair("default", SCI_PARSER_STAT_DEFAULT));
    gs_pSyntaxMap->insert(make_pair("addr", SCI_PARSER_STAT_ADDR));
    gs_pSyntaxMap->insert(make_pair("register", SCI_PARSER_STAT_REGISTER));
    gs_pSyntaxMap->insert(make_pair("error", SCI_PARSER_STAT_ERROR));
    gs_pSyntaxMap->insert(make_pair("message", SCI_PARSER_STAT_MESSAGE));
    gs_pSyntaxMap->insert(make_pair("hex", SCI_PARSER_STAT_HEX));
    gs_pSyntaxMap->insert(make_pair("data", SCI_PARSER_STAT_DATA));
    gs_pSyntaxMap->insert(make_pair("byte", SCI_PARSER_STAT_BYTE));
    gs_pSyntaxMap->insert(make_pair("inst", SCI_PARSER_STAT_INST));
    gs_pSyntaxMap->insert(make_pair("call", SCI_PARSER_STAT_CALL));
    gs_pSyntaxMap->insert(make_pair("jmp", SCI_PARSER_STAT_JMP));
    gs_pSyntaxMap->insert(make_pair("proc", SCI_PARSER_STAT_PROC));
    gs_pSyntaxMap->insert(make_pair("module", SCI_PARSER_STAT_MODULE));
    gs_pSyntaxMap->insert(make_pair("param", SCI_PARSER_STAT_PARAM));
    gs_pSyntaxMap->insert(make_pair("keyword", SCI_PARSER_STAT_KEYWORD));
    gs_pSyntaxMap->insert(make_pair("number", SCI_PARSER_STAT_NUMBER));
    gs_pSyntaxMap->insert(make_pair("hight", SCI_PARSER_STAT_HIGHT));
}

BOOL UpdateSyntaxView(SyntaxView *pSyntaxView) {
    pSyntaxView->SetDefStyle(gs_defTextColour, gs_defBackColour);
    pSyntaxView->ShowCaretLine(true, gs_CaretLineColour);

    map<int, SyntaxColourDesc *>::const_iterator it;
    for (it = gs_pSyntaxCfg->begin() ; it != gs_pSyntaxCfg->end() ; it++)
    {
        SyntaxColourDesc *desc = it->second;
        pSyntaxView->SetStyle(it->first, desc->m_dwTextColour, desc->m_dwBackColour);
    }
    return TRUE;
}

BOOL LoadSyntaxCfg(const string &path)
{
    if (!gs_bInit)
    {
        gs_bInit = true;

        _InitSyntaxCfg();
    }

    fstream fp(path.c_str());
    if (!fp.is_open())
    {
        return FALSE;
    }

    Value content;
    Reader().parse(fp, content);
    if (content.type() != objectValue)
    {
        return FALSE;
    }

    //Global Config
    Value global = content["globalCfg"];
    Value cfg = content["syntaxCfg"];
    gs_defTextColour = GetColourFromStr(global["defTextColour"].asCString());
    gs_defBackColour = GetColourFromStr(global["defBackColour"].asCString());
    gs_CaretLineColour = GetColourFromStr(global["curLineColour"].asCString());

    vector<string> vMembers = cfg.getMemberNames();
    vector<string>::const_iterator it;
    for (it = vMembers.begin() ; it != vMembers.end() ; it++)
    {
        SyntaxColourDesc *desc = _GetDescFromJson(cfg[it->c_str()]);
        desc->m_strDesc = *it;

        map<mstring, int>::const_iterator ij = gs_pSyntaxMap->find(*it);
        if (ij != gs_pSyntaxMap->end())
        {
            gs_pSyntaxCfg->insert(make_pair(ij->second, desc));
        }
    }
    return TRUE;
}