#include "CmdQueue.h"

CCmdQueue::CCmdQueue() : m_dwCurPos(0)
{}

CCmdQueue::~CCmdQueue()
{}

void CCmdQueue::EnterCmd(const ustring &wstrCmd)
{
    ustring wstr(wstrCmd);
    wstr.trim();

    set<ustring>::const_iterator it;
    if (m_vCmdHash.end() != (it = m_vCmdHash.find(wstr)))
    {
        for (vector<ustring>::const_iterator itCmd = m_vCmdList.begin() ; itCmd != m_vCmdList.end() ; itCmd++)
        {
            if (*itCmd == wstr)
            {
                m_vCmdList.erase(itCmd);
                break;
            }
        }
    }
    else
    {
        m_vCmdHash.insert(wstr);
    }
    m_vCmdList.push_back(wstr);
    m_dwCurPos = - 1;
}

ustring CCmdQueue::GetFrontCmd()
{
    if (m_vCmdList.empty())
    {
        return L"";
    }

    if (-1 == m_dwCurPos)
    {
        m_dwCurPos = (DWORD)m_vCmdList.size() - 1;
        return m_vCmdList[m_dwCurPos];
    }

    if (m_dwCurPos)
    {
        return m_vCmdList[--m_dwCurPos];
    }
    return m_vCmdList[0];
}

ustring CCmdQueue::GetLastCmd()
{
    if (m_vCmdList.empty() || -1 == m_dwCurPos)
    {
        return L"";
    }

    if (m_dwCurPos < (m_vCmdList.size() - 1))
    {
        return m_vCmdList[++m_dwCurPos];
    }
    return m_vCmdList[m_vCmdList.size() - 1];
}