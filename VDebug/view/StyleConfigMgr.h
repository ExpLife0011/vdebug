#pragma once
#include <Windows.h>
#include <map>
#include <list>
#include "../../ComLib/mstring.h"

struct StyleConfigNode {
    std::mstring mStyleDesc;
    int mSyntaxStyle;
    DWORD mRgbText;
    DWORD mRgbBack;

    bool operator==(const StyleConfigNode &info) const {
        bool res = (
            mStyleDesc == info.mStyleDesc &&
            mSyntaxStyle == info.mSyntaxStyle &&
            mRgbText == info.mRgbText &&
            mRgbBack == info.mRgbBack
            );
        return res;
    }

    bool operator!=(const StyleConfigNode &info) const {
        bool res = (
            mStyleDesc != info.mStyleDesc ||
            mSyntaxStyle != info.mSyntaxStyle ||
            mRgbText != info.mRgbText ||
            mRgbBack != info.mRgbBack
            );
        return res;
    }

    StyleConfigNode() {
        mSyntaxStyle = -1;
        mRgbText = -1, mRgbBack = - 1;
    }
};

struct StyleConfigInfo {
    std::map<std::mstring, StyleConfigNode> mCfgSet;//��ʽ��ʽ�б�
    bool mLineNum;              //�Ƿ�չʾ�к�
    std::mstring mFontName;     //��������
    int mFontSize;              //�����С
    DWORD mSelColour;           //ѡ���������ɫ
    DWORD mSelAlpha;            //ѡ�������͸����

    bool operator!=(const StyleConfigInfo &info) const {
        if (mCfgSet != info.mCfgSet || mLineNum != info.mLineNum)
        {
            return true;
        }

        if (mFontName != info.mFontName || mFontSize != info.mFontSize)
        {
            return true;
        }

        if (mSelColour != info.mSelColour || mSelAlpha != info.mSelAlpha)
        {
            return true;
        }
        return false;
    }

    StyleConfigInfo() {
        mLineNum = false, mFontSize = 10;
        mSelColour = 0, mSelAlpha = 0;
    }
};

class CStyleConfig {
public:
    CStyleConfig();
    CStyleConfig(const CStyleConfig &dst);
    virtual ~CStyleConfig();
    void SetDefault();
    bool LoadCache(const std::mstring &filePath);
    bool SaveCache(const std::mstring &filePath) const;
    StyleConfigInfo GetStyleConfig() const;
    void UpdateStyleConfig(const StyleConfigInfo &cfg);
private:
    void ClearCache();
    std::mstring GetRgbStr(DWORD rgb) const;
    DWORD GetRgbFromStr(const std::mstring &str) const;

private:
    StyleConfigInfo mStyleInfo;
};