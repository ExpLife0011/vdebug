#include "StyleConfigMgr.h"
#include "../../ComLib/json/json.h"
#include "../../ComLib/StrUtil.h"
#include "../SyntaxHlpr/SyntaxDef.h"
#include <fstream>

using namespace std;
using namespace Json;

CStyleConfig::CStyleConfig() {
    SetDefault();
}

CStyleConfig::CStyleConfig(const CStyleConfig &dst) {
    mStyleInfo = dst.mStyleInfo;
}

CStyleConfig::~CStyleConfig() {
}

//×Ö·û´®RGB(11,22,44) >> DWORD
DWORD CStyleConfig::GetRgbFromStr(const mstring &tmp) const {
    mstring str(tmp);
    str.trim();
    str.delchar(' ');

    size_t pos1 = str.find("(");
    size_t pos2 = str.rfind(")");

    if (mstring::npos == pos1 || mstring::npos == pos2 || pos2 <= pos1)
    {
        return -1;
    }

    mstring numStr = str.substr(pos1 + 1, pos2 - pos1 - 1);
    WORD r, g, b;
    pos1 = 0;
    pos2 = numStr.find(",");
    r = atoi(numStr.substr(pos1, pos2 - pos1).c_str());
    pos1 = pos2 + 1;
    pos2 = numStr.find(",", pos1);
    g = atoi(numStr.substr(pos1, pos2 - pos1).c_str());
    pos1 = pos2 + 1;
    b = atoi(numStr.substr(pos1, numStr.size() - pos1).c_str());
    return RGB(r, g, b);
}

bool CStyleConfig::LoadCache(const mstring &filePath) {
    SetDefault();
    fstream fp(filePath.c_str());

    if (!fp.is_open())
    {
        return false;
    }

    Value root;
    Reader().parse(fp, root);
    if (root.type() != objectValue || root["global"].type() != objectValue)
    {
        return false;
    }

    vector<string> set1 = root.getMemberNames();
    for (size_t i = 0 ; i < set1.size() ; i++)
    {
        const string &name = set1[i];
        const Value &tmp = root.get(name, Value());

        if (name == "global")
        {
            mStyleInfo.mFontName = tmp["fontName"].asString();
            mStyleInfo.mFontSize = tmp["fontSize"].asInt();
            mStyleInfo.mLineNum = tmp["lineNum"].asBool();
            mStyleInfo.mSelColour = GetRgbFromStr(tmp["selColour"].asString());
            mStyleInfo.mSelAlpha = tmp["selAlpha"].asInt();
        } else {
            StyleConfigNode node;
            node.mStyleDesc = name;
            node.mSyntaxStyle = tmp["style"].asInt();
            node.mRgbText = GetRgbFromStr(tmp["textColour"].asString());
            node.mRgbBack = GetRgbFromStr(tmp["backColour"].asString());
            mStyleInfo.mCfgSet[name] = node;
        }
    }
    return true;
}

mstring CStyleConfig::GetRgbStr(DWORD rgb) const {
    return FormatA("RGB(%d,%d,%d)", (int)GetRValue(rgb), (int)GetGValue(rgb), (int)GetBValue(rgb));
}

/*
{
    "global": {
        "font":"Courier New",
        "lineNum":0,
        "selColour":"RGB(73,107,205)",
        "selAlpha":150
    },
    "default": {
        "style":101,
        "textColour":"RGB(255, 255, 255)",
        "backColour":"RGB(0, 0, 0)"
    },
    "cmdSend": {
        "style":107,
        "textColour":"RGB(86, 156, 214)",
        "backColour":"RGB(0, 0, 0)"
    },
    "cmdRecv": {
        "style":108,
        "textColour":"RGB(0, 0xff, 0)",
        "backColour":"RGB(0, 0, 0)"
    }
}
*/
bool CStyleConfig::SaveCache(const mstring &filePath) const {
    Value content;

    const map<mstring, StyleConfigNode> &cfgSet = mStyleInfo.mCfgSet;
    for (map<mstring, StyleConfigNode>::const_iterator it = cfgSet.begin() ; it != cfgSet.end() ; it++)
    {
        Value node;
        node["style"] = it->second.mSyntaxStyle;
        node["textColour"] = GetRgbStr(it->second.mRgbText);
        node["backColour"] = GetRgbStr(it->second.mRgbBack);

        content[it->second.mStyleDesc] = node;
    }

    Value global;
    global["fontName"] = mStyleInfo.mFontName;
    global["fontSize"] = mStyleInfo.mFontSize;
    global["lineNum"] = mStyleInfo.mLineNum;
    global["selColour"] = GetRgbStr(mStyleInfo.mSelColour);
    global["selAlpha"] = (int)mStyleInfo.mSelAlpha;
    content["global"] = global;

    DeleteFileA(filePath.c_str());

    fstream fp(filePath.c_str(), ios::trunc | ios::out);
    if (!fp.is_open())
    {
        return false;
    }

    fp << StyledWriter().write(content);
    fp.close();
    return true;
}

StyleConfigInfo CStyleConfig::GetStyleConfig() const {
    return mStyleInfo;
}

void CStyleConfig::UpdateStyleConfig(const StyleConfigInfo &cfg) {
    mStyleInfo = cfg;
}

void CStyleConfig::SetDefault() {
    mStyleInfo.mFontName = "Courier New";
    mStyleInfo.mFontSize = 10;
    mStyleInfo.mLineNum = false;

    StyleConfigNode node;
    node.mStyleDesc = "default";
    node.mRgbText = RGB(255, 236, 1);
    node.mRgbBack = RGB(0, 0, 0);
    node.mSyntaxStyle = STYLE_CMD_DEFAULT;
    mStyleInfo.mCfgSet[node.mStyleDesc] = node;

    node.mStyleDesc = "cmdSend";
    node.mRgbText = RGB(187, 255, 255);
    node.mRgbBack = RGB(0, 0, 0);
    node.mSyntaxStyle = STYLE_CMD_SEND;
    mStyleInfo.mCfgSet[node.mStyleDesc] = node;

    node.mStyleDesc = "cmdRecv";
    node.mRgbText = RGB(255, 236, 1);
    node.mRgbBack = RGB(0, 0, 0);
    node.mSyntaxStyle = STYLE_CMD_RECV;
    mStyleInfo.mCfgSet[node.mStyleDesc] = node;

    node.mStyleDesc = "cmdHight";
    node.mRgbText = RGB(0, 245, 255);
    node.mRgbBack = RGB(0, 0, 0);
    node.mSyntaxStyle = STYLE_CMD_HIGHT;
    mStyleInfo.mCfgSet[node.mStyleDesc] = node;

    mStyleInfo.mSelColour = RGB(255, 255, 255);
    mStyleInfo.mSelAlpha = 120;
}