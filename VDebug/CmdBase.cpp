#include <Windows.h>
#include <Shlwapi.h>
#include "CmdBase.h"

CCmdBase::CCmdBase()
{}

CCmdBase::~CCmdBase()
{}

DbgCmdResult CCmdBase::RunCommand(const ustring &wstrCmd, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult res;
    ustring wstr(wstrCmd);
    wstr.makelower();
    wstr.trim();
    if (wstr.empty())
    {
        return res;
    }

    ustring wstrStart;
    ustring wstrParam;
    size_t i = wstr.find(L" ");
    if (ustring::npos == i)
    {
        wstrStart = wstr;
    }
    else
    {
        wstrStart = wstr.substr(0, i);
        wstrParam = wstr.c_str() + i;
        wstrParam.trim();
    }
    res = OnCommand(wstrStart, wstrParam, bShow, pParam);
    return res;
}

BOOL CCmdBase::InsertFunMsg(const ustring &wstrIndex, const DbgFunInfo &vProcInfo)
{
    m_vProcMap[wstrIndex] = vProcInfo;
    return TRUE;
}

DWORD64 CCmdBase::GetFunAddr(const ustring &wstr)
{
    ustring wstrContent(wstr);
    wstrContent.makelower();
    wstrContent.trim();
    ustring wstrFun;
    ustring wstrOffset;
    size_t pos = 0;
    if (ustring::npos != (pos = wstrContent.find(L"+")))
    {
        wstrFun = wstrContent.substr(0, pos);
        wstrOffset = (wstrContent.c_str() + pos + 1);
    }
    else
    {
        wstrFun = wstrContent;
    }

    DWORD64 dwOffset = 0;
    GetNumFromStr(wstrOffset, dwOffset);
    map<ustring, DbgFunInfo>::const_iterator it;
    if (m_vProcMap.end() == (it = m_vProcMap.find(wstrFun)))
    {
        return 0;
    }
    return (it->second.m_dwProcAddr + dwOffset);
}

//0x43fdad12
//0n12232433
//5454546455
BOOL CCmdBase::GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwResult)
{
    return StrToInt64ExW(wstrNumber.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
}

DbgCmdResult CCmdBase::OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    return DbgCmdResult();
}