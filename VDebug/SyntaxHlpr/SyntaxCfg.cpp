#include <Windows.h>
#include <map>
#include <string>
#include <fstream>
#include "SyntaxParser.h"
#include <ComStatic/ComStatic.h>
#include "SyntaxCfg.h"
#include <ComLib/ComLib.h>

using namespace std;

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

static SyntaxColourDesc *_GetDescFromJson(const cJSON *json)
{
    SyntaxColourDesc *desc = new SyntaxColourDesc();
    desc->m_dwTextColour = GetColourFromStr(cJSON_GetObjectItem(json, "textColour")->valuestring);
    desc->m_dwBackColour = GetColourFromStr(cJSON_GetObjectItem(json, "backColour")->valuestring);
    desc->m_bBold = cJSON_GetObjectItem(json, "bold")->valueint;
    desc->m_bItalic = cJSON_GetObjectItem(json, "italic")->valueint;

    if (desc->m_dwTextColour == NULL_COLOUR)
    {
        desc->m_dwTextColour = gs_defTextColour;
    }

    if (desc->m_dwBackColour == NULL_COLOUR)
    {
        desc->m_dwBackColour = gs_defBackColour;
    }
    desc->m_strDesc = json->string;
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

BOOL LoadSyntaxCfg(const wstring &path)
{
    if (!gs_bInit)
    {
        gs_bInit = true;

        _InitSyntaxCfg();
    }

    cJSON *root = NULL;
    PFILE_MAPPING_STRUCT pMapping = NULL;
    BOOL stat = FALSE;

    do 
    {
        pMapping = MappingFileW(path.c_str());
        if (pMapping == NULL || pMapping->hFile == INVALID_HANDLE_VALUE)
        {
            break;
        }

        cJSON *root = cJSON_Parse((const char *)pMapping->lpView);
        if (!root || root->type != cJSON_Object)
        {
            break;
        }


        //Global Config
        cJSON *golbal = cJSON_GetObjectItem(root, "globalCfg");
        cJSON *cfg = cJSON_GetObjectItem(root, "syntaxCfg");

        gs_defTextColour = cJSON_GetObjectItem(golbal, "defTextColour")->valueint;
        gs_defBackColour = cJSON_GetObjectItem(golbal, "defBackColour")->valueint;
        gs_CaretLineColour = cJSON_GetObjectItem(golbal, "curLineColour")->valueint;

        for (cJSON *it = cfg ; it != NULL ; it = it->next) {
            SyntaxColourDesc *desc = _GetDescFromJson(it);

            map<mstring, int>::const_iterator ij = gs_pSyntaxMap->find(desc->m_strDesc);
            if (ij != gs_pSyntaxMap->end())
            {
                gs_pSyntaxCfg->insert(make_pair(ij->second, desc));
            }
        }
    } while (FALSE);

    if (pMapping)
    {
        CloseFileMapping(pMapping);
    }

    if (root)
    {
        cJSON_Delete(root);
    }
    return stat;
}