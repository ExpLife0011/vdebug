#include "BreakPoint.h"
//#include "common.h"
//#include "MainView.h"
#include "ProcDbg.h"
//#include <SyntaxHlpr/SyntaxView.h>
//#include <SyntaxHlpr/SyntaxParser.h>
#include <ComLib/ComLib.h>
#include <mq/mq.h>

DWORD CBreakPointMgr::msSerial = 0;

CBreakPointMgr::CBreakPointMgr()
{}

CBreakPointMgr::~CBreakPointMgr()
{}

void CBreakPointMgr::Int3BpCallback()
{
    TITAN_ENGINE_CONTEXT_t context = CProcDbgger::GetInstance()->GetCurrentContext();

    BreakPointInfo bp;
    if (GetBreakPointMgr()->GetBpByAddr(context.cip, bp))
    {
        if (bp.mBpStat != em_bp_enable)
        {
            return;
        }
        

        Value result;
        result["addr"] = FormatA("0x%08x", (DWORD)bp.mBpAddr);
        result["symbol"] = bp.mSymbol;
        result["tid"] = (int)((DEBUG_EVENT*)GetDebugData())->dwThreadId;

        EventDbgInfo eventInfo;
        eventInfo.mEventType = DBG_EVENT_USER_BREAKPOINT;
        eventInfo.mEventResult = result;
        eventInfo.mEventLabel = SCI_LABEL_DEFAULT;
        eventInfo.mEventShow = FormatA("触发用户断点 %hs %hs\n", result["addr"].asString().c_str(), bp.mSymbol.c_str());

        MsgSend(MQ_CHANNEL_DBG_SERVER, MakeEventRequest(eventInfo).c_str());
        CProcDbgger::GetInstance()->Wait();
    } else {
    }
}

bool CBreakPointMgr::IsBpInCache(DWORD64 dwAddr) const
{
    for (vector<BreakPointInfo>::const_iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (it->mBpAddr == dwAddr)
        {
            return true;
        }
    }
    return false;
}

bool CBreakPointMgr::DeleteInCache(DWORD64 dwAddr)
{
    for (vector<BreakPointInfo>::iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (it->mBpAddr == dwAddr)
        {
            mBreakPoints.erase(it);
            return true;
        }
    }
    return false;
}

BOOL CBreakPointMgr::GetBpByAddr(DWORD64 addr, BreakPointInfo &info) const {
    for (vector<BreakPointInfo>::const_iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (it->mBpAddr == addr)
        {
            info = *it;
            return TRUE;
        }
    }
    return FALSE;
}

bool CBreakPointMgr::PushBreakPoint(const BreakPointInfo &info)
{
    mBreakPoints.push_back(info);
    return true;
}

BOOL CBreakPointMgr::SetBreakPoint(DWORD64 dwAddr, const CmdUserParam *pUserContxt)
{
    if (IsBpInCache(dwAddr))
    {
        mLastErr = FormatA("位于 0x%08x 的断点已存在");
        return FALSE;
    }

    if (SetBPX((ULONG_PTR)dwAddr, UE_BREAKPOINT, Int3BpCallback))
    {
        BreakPointInfo point;
        point.mBpAddr = dwAddr;
        point.mSymbol = CProcDbgger::GetInstance()->GetSymFromAddr((void *)dwAddr);
        point.mSerial = msSerial++;
        point.mBpType = em_breakpoint_int3;

        if (pUserContxt)
        {
            memcpy(&point.mUserContext, pUserContxt, sizeof(CmdUserParam));
        }
        PushBreakPoint(point);
        return TRUE;
    }
    mLastErr = FormatA("设置 0x%08x 处的断点失败");
    return FALSE;
}

BOOL CBreakPointMgr::DeleteBpByIndex(int index)
{
    for (vector<BreakPointInfo>::const_iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (it->mSerial == index)
        {
            DeleteBPX((ULONG_PTR)it->mBpAddr);
            DeleteInCache(it->mBpAddr);
            return TRUE;
        }
    }
    mLastErr = FormatA("未找到序号为 %d 的断点", index);
    return FALSE;
}

BOOL CBreakPointMgr::DisableBpByIndex(int index) {
    for (vector<BreakPointInfo>::iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (it->mSerial == index)
        {
            it->mBpStat = em_bp_disable;
            DisableBPX((ULONG_PTR)it->mBpAddr);
            return TRUE;
        }
    }
    mLastErr = FormatA("未找到序号为 %d 的断点", index);
    return FALSE;
}

BOOL CBreakPointMgr::EnableBpByIndex(int index) {
    for (vector<BreakPointInfo>::iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (it->mSerial == index)
        {
            it->mBpStat = em_bp_enable;
            EnableBPX((ULONG_PTR)it->mBpAddr);
            return TRUE;
        }
    }
    mLastErr = FormatA("未找到序号为 %d 的断点", index);
    return FALSE;
}

BOOL CBreakPointMgr::DeleteAllBp()
{
    for (vector<BreakPointInfo>::const_iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        DeleteBPX((ULONG_PTR)it->mBpAddr);
    }
    mBreakPoints.clear();
    return TRUE;
}

vector<BreakPointInfo> CBreakPointMgr::GetBpSet() const {
    return mBreakPoints;
}

std::mstring CBreakPointMgr::GetLastErr() const {
    return mLastErr;
}

BOOL CBreakPointMgr::OnBreakPoint(DWORD64 dwAddr)
{
    for (vector<BreakPointInfo>::const_iterator it = mBreakPoints.begin() ; it != mBreakPoints.end() ; it++)
    {
        if (dwAddr == it->mBpAddr)
        {
            //GetSyntaxView()->AppendText(SCI_LABEL_DEFAULT, FormatA("断点%ls已触发", it->m_wstrName.c_str()));
            //GetCurrentDbgger()->RunCommand("r");

            if (it->mUserContext.m_pfnCallback)
            {
                it->mUserContext.m_pfnCallback(it->mUserContext.m_pParam, NULL);
            }
            return TRUE;
        }
    }
    return TRUE;
}

CBreakPointMgr *GetBreakPointMgr()
{
    static CBreakPointMgr *s_ptr = NULL;

    if (!s_ptr)
    {
        s_ptr = new CBreakPointMgr;
    }
    return s_ptr;
}