#ifndef DBGPROTOCOL_DBGCTRL_H_H_
#define DBGPROTOCOL_DBGCTRL_H_H_

/*
Э�鰴Ӧ�ó�����������࣬һ��һ���Ӧ��ʽ�͵���������ϱ�
һ��һ���Ӧ��ʽ
���󷽣�
{
    "type":"ctrl",
    "cmd":"attach",
    "content": {
        "pid":1234
    }
}

Ӧ�𷽣�
{
    "status":0,
    "label":"default",
    "show":"showmsg",
    "result":{
        ...
    }
}

������������
{
    "type":"event",
    "event":"moduleload",
    "label":"default",
    "show":"xxxx ģ�����",
    "content":{
    }
}

����������Ϊctrl������
{
    "type":"ctrl",
    "cmd":"RunCmd",
    "content": {
        "command":"kv"
    }
}
*/
//���ӷ�ʽ����
#define DBG_CTRL_ATTACH              "attach"
//ִ�п�ִ���ļ�
#define DBG_CTRL_EXEC                "exec"
//���������
#define DBG_CTRL_DETACH              "ctrlDetach"
//ִ�е�������
#define DBG_CTRL_RUNCMD              "RunCmd"
//���Կ��ƶ˻�ȡ������Ϣ
#define DBG_CTRL_GET_PROC            "GetProcInfo"
//����dump�ļ�
#define DBG_CTRL_OPEN_DUMP           "OpenDump"
//Ϊ��������dump�ļ�
#define DBG_CTRL_DUMP_PROC           "DumpProc"
//�жϵ�����
#define DBG_CTRL_BREAK               "BreakDebugger"

//��������¼��
#define DBG_CTRL_TEST_DESC           "DescTest"
#define DBG_CTRL_INPUT_DESC          "DescSave"
/*********************���Կ���ָ�����******************************/

/*********************�����¼���ʼ**********************************/
#define DBG_DBG_EVENT                "event"
//������Ϣ�¼�
#define DBG_EVENT_MSG                "dbgmsg"
//���̴����¼�
#define DBG_EVENT_DBG_PROC_CREATE    "proccreate"
//�����˳��¼�
#define DBG_EVENT_DBG_PROC_END       "procend"
//���Խ���ִ���¼�
#define DBG_EVENT_DBG_PROC_RUNNING   "procRunning"
//ģ������¼�
#define DBG_EVENT_MODULE_LOAD        "moduleload"
//ģ��ж���¼�
#define DBG_EVENT_MODULE_UNLOAD      "moduelunload"
//ֹͣ�����¼�
#define DBG_EVENT_DETACH             "eventDetach"
//�쳣�¼��ַ�
#define DBG_EVENT_EXCEPTION          "exception"
//���̱仯�¼�
#define DBG_EVENT_PROC_CHANGED       "ProcChanged"
//ϵͳ�ж��¼�
#define DBG_EVENT_SYSTEM_BREAKPOINT  "SyatemBreakpoint"
//�û��ϵ��¼�
#define DBG_EVENT_USER_BREAKPOINT    "UserBreakpoint"
/*********************�����¼�����**********************************/
#endif //DBGPROTOCOL_DBGCTRL_H_H_