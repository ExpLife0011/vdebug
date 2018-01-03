#include "SyntaxDesc.h"

SyntaxDesc::SyntaxDesc(const ustring &wstrContent, const SyntaxColourDesc &colour)
{
    SyntaxColourNode node(wstrContent, 0, colour);
    vector<SyntaxColourNode> v;
    v.push_back(node);
    m_vSyntaxDesc.push_back(v);
    m_vShowInfo.push_back(wstrContent);
}

SyntaxDesc::SyntaxDesc(const ustring &wstrContent)
{
    SyntaxColourNode node(wstrContent, 0, COLOUR_MSG);
    vector<SyntaxColourNode> v;
    v.push_back(node);
    m_vSyntaxDesc.push_back(v);
    m_vShowInfo.push_back(wstrContent);
}

SyntaxDesc::SyntaxDesc()
{}
