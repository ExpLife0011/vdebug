#ifndef CMD_QUEUE_H_H_
#define CMD_QUEUE_H_H_
#include <Windows.h>
#include <vector>
#include <set>
#include <ComLib/ComLib.h>

//����ؼ����·�ҳ����
//�㷨��
//��������б����У�����������Ϊ����һ��,���Ѵ��ڵ�ɾ��
//��������б���û�У�����������Ϊ����һ��
//ÿ�������posָ������һ��
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