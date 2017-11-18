#ifndef CMD_QUEUE_H_H_
#define CMD_QUEUE_H_H_
#include <Windows.h>
#include <vector>
#include <set>
#include "mstring.h"

using namespace std;

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

    void EnterCmd(const ustring &wstrCmd);

    ustring GetFrontCmd();

    ustring GetLastCmd();
protected:
    DWORD m_dwCurPos;
    vector<ustring> m_vCmdList;
    set<ustring> m_vCmdHash;
};

#endif