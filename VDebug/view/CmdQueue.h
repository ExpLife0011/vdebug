#ifndef CMD_QUEUE_H_H_
#define CMD_QUEUE_H_H_
#include <Windows.h>
#include <vector>
#include <set>
#include "mstring.h"

using namespace std;

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

    void EnterCmd(const ustring &wstrCmd);

    ustring GetFrontCmd();

    ustring GetLastCmd();
protected:
    DWORD m_dwCurPos;
    vector<ustring> m_vCmdList;
    set<ustring> m_vCmdHash;
};

#endif