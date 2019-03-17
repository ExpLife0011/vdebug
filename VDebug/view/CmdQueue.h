#ifndef CMD_QUEUE_H_H_
#define CMD_QUEUE_H_H_
#include <Windows.h>
#include <vector>
#include <set>
#include <ComLib/ComLib.h>

//命令控件上下翻页缓存
//算法：
//如果命令列表中有，将该命令置为最新一条,将已存在的删除
//如果命令列表中没有，将该命令置为最新一条
//每次输入后，pos指向最新一条
class CCmdQueue
{
public:
    CCmdQueue();

    virtual ~CCmdQueue();

    void EnterCmd(const std::mstring &cmd);

    std::mstring GetFrontCmd();

    std::mstring GetLastCmd();
protected:
    DWORD m_dwCurPos;
    std::vector<std::mstring> m_vCmdList;
    std::set<std::mstring> m_vCmdHash;
};

#endif