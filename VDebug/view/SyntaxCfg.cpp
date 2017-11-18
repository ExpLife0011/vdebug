#include <Windows.h>
#include <map>
#include "json.h"
#include "mstring.h"
#include "common.h"
#include "SyntaxCfg.h"

using namespace std;
using namespace Json;

static map<ustring, SyntaxColourDesc> *gs_pSyntaxCfg = new map<ustring, SyntaxColourDesc>();

SyntaxColourDesc GetSyntaxCfg(LPCWSTR wszName)
{
    if (!wszName || !wszName[0])
    {
        return SyntaxColourDesc();
    }

    if (gs_pSyntaxCfg->end() == gs_pSyntaxCfg->find(wszName))
    {
        return SyntaxColourDesc();
    }
    return (*gs_pSyntaxCfg)[wszName];
}

static SyntaxColourDesc _GetDescFromJson(const Value &vJson)
{
    SyntaxColourDesc vDesc;
    vDesc.m_dwTextColour = GetColourFromStr(JsonGetStrValue(vJson, "textColour").c_str());
    vDesc.m_dwBackColour = GetColourFromStr(JsonGetStrValue(vJson, "backColour").c_str());
    vDesc.m_bBold = JsonGetIntValue(vJson, "bold");
    vDesc.m_bItalic = JsonGetIntValue(vJson, "italic");
    return vDesc;
}

BOOL LoadSyntaxCfg(const Value &vSyntaxCfg)
{
    vector<string> vMembers = vSyntaxCfg.getMemberNames();
    vector<string>::const_iterator it;
    for (it = vMembers.begin() ; it != vMembers.end() ; it++)
    {
        SyntaxColourDesc vDesc = _GetDescFromJson(vSyntaxCfg[it->c_str()]);
        (*gs_pSyntaxCfg)[UtoW(*it)] = vDesc;
    }
    return TRUE;
}