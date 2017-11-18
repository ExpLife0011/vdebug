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

BOOL CCmdBase::AddProcMsg(const ustring &wstrIdex, DWORD64 dwAddr)
{
    ustring wstr = wstrIdex;
    wstr.makelower();
    m_vProcMap[wstr] = dwAddr;
    return TRUE;
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