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
#define DBG_CTRL_ATTACH              "attach"

/*
{
    "cmd":"exec",
    "content":{
        "path":"abcdef.exe",
        "param":"abcdefgh"
    }
}
*/
#define DBG_CTRL_EXEC                "exec"

//{}
#define DBG_CTRL_DETACH              "ctrlDetach"


/*
{
    "cmd":"RunCmd",
    "content":{
        "mode":1,                          //1:������չʾ�ַ��� 2:����Json��ʽ��ִ�н��
        "cmd":"bp kernen32!CreateFileW"    //cmd ����
    }
}

{
    "cmd": "reply",
    "content": {
        "status": 0,
        "reason": "abcdef",
        "result":{
            "cmdCode":0,
            "mode":1,
            "cmdLabel":"CallStack",                     //չʾ��ǩ
            "cmdShow":"abcd1234",                       //չʾ����
            "cmdResult": [{
                "addr": "0x0xabcd12ff",
                "function":"kernel32!CreateFileW",
                "param0": "0xabcd1234",
                "param1": "0xabcd1234",
                "param2": "0xabcd1233",
                "param3": "0xabcd1233"
            }]
        }
    }
}
*/
#define DBG_CTRL_RUNCMD               "RunCmd"

/*
{
    "cmd":"GetProcInfo",
    "content":{
        "start":1
    }
}
*/
#define DBG_CTRL_GET_PROC            "GetProcInfo"

/*
{
    "cmd":"OpenDump",
    "content":{
        "dumpPath":"d:\\abcdef.dmp"
    }
}

{
    "cmd": "reply",
    "content": {
        "status": 0,
        "reason": "abcdef",
    }
}
*/
#define DBG_CTRL_OPEN_DUMP           "OpenDump"

/*
{
    "cmd":"DumpProc",
    "content":{
        "pid":1122
    }
}

{
    "cmd": "reply",
    "content": {
        "status": 0,
        "reason": "abcdef"
    }
}
*/
#define DBG_CTRL_DUMP_PROC           "DumpProc"

//�жϵ�����
#define DBG_CTRL_BREAK               "BreakDebugger"
/*********************���Կ���ָ�����******************************/

/*********************�����¼���ʼ**********************************/
#define DBG_DBG_EVENT                   "event"

/*
{
    "cmd":"event",
    "content":{
        "eventType":"DbgMessage",
        "mode":1,                            //1:չʾ��Ϣ��2:�����Ϣ
        "eventLabel":"DbgMsg",               //չʾ��ǩ
        "eventShow":"abcd1234",              //չʾ����
        "eventResult": {
            ��msg��:"abcdefgh"
        }
}
*/
#define DBG_EVENT_MSG                    "dbgmsg"

/*
{
    "cmd":"event",
    "content":{
        "eventType":"proccreate",
        "mode":1,                            //1:չʾ��Ϣ��2:�����Ϣ
        "eventLabel":"procCreate",           //չʾ��ǩ
        "eventShow":"abcd1234",              //չʾ����
        "eventResult": {
            "pid":1234,
            "image":"d:\\desktop\\1234.exe",
            "baseAddr":"0x4344353",
            "entryAddr":"0x4344389"
        }
}
*/
#define DBG_EVENT_DBG_PROC_CREATE        "proccreate"
#define DBG_EVENT_DBG_PROC_END           "procend"

/*
{
    "cmd":"event",
    "content":{
        "eventType":"procRunning",
        "mode":1,                            //0:չʾ��Ϣ��1:�����Ϣ
        "eventLabel":"Default",              //չʾ��ǩ
        "eventShow":"abcd1234",              //չʾ����
        "eventResult": {
        }
}
*/
#define DBG_EVENT_DBG_PROC_RUNNING       "procRunning"          //���Խ���ִ����

/*
{
    "cmd":"event",
    "content":{
        "eventType":"moduleload",
        "mode":1,                                           //1:չʾ��Ϣ��2:�����Ϣ
        "eventLabel":"Default",                             //չʾ��ǩ
        "eventShow":"0xffaabbcc 0x11223344 kernel32.dll",   //չʾ����
        "eventResult": {
            "name":"kernel32.dll",
            "baseAddr":"0x4344353",
            "endAddr":"0x43443ff"
        }
}
*/
#define DBG_EVENT_MODULE_LOAD        "moduleload"
#define DBG_EVENT_MODULE_UNLOAD      "moduelunload"
#define DBG_EVENT_MODULE_LOADA       "moduleload"
#define DBG_EVENT_MODULE_UNLOADA     "moduelunload"
#define DBG_EVENT_DETACH             "eventDetach"

/*
{
    "cmd":"event",
    "content":{
        "eventType":"exception",
        "mode":1,                                           //1:չʾ��Ϣ��2:�����Ϣ
        "eventLabel":"Default",                             //չʾ��ǩ
        "eventShow":"�������״β��񵽸��쳣��Ϣ",              //չʾ����
        "eventResult": {
        }
}
*/
#define DBG_EVENT_EXCEPTION          "exception"

/*
{
    "cmd":"event",
    "content":{
        "eventType":"proc_add",
        "mode":1,
        "data":{
            "add":[
                {
                    "unique":12345,
                    "pid":1234,
                    "procPath":"d:\\abcdef.exe",
                    "procDesc":"desc",
                    "cmd":"abcdef",
                    "startTime":"2018-11-11 11:11:11:123",
                    "x64":1,
                    "session":1,
                    "user":"DESKTOP-DCTRL5K\\Administrator",
                    "sid":"S-1-5-21-2669793992-3689076831-3814312677-500"
                },
                ...
            ],
            "kill":[
                1111,2222,3333
            ]
        }
    }
}

{
    "cmd":"event",
    "content":{
        "eventType":"proc_add",
        "mode":1,                                           //1:չʾ��Ϣ��2:�����Ϣ
        "eventLabel":"Default",                             //չʾ��ǩ
        "eventShow":"0xffaabbcc 0x11223344 kernel32.dll",   //չʾ����
        "eventResult": {
            "add":[
                {
                    "unique":12345,
                    "pid":1234,
                    "procPath":"d:\\abcdef.exe",
                    "procDesc":"desc",
                    "cmd":"abcdef",
                    "startTime":"2018-11-11 11:11:11:123",
                    "x64":1,
                    "session":1,
                    "user":"DESKTOP-DCTRL5K\\Administrator",
                    "sid":"S-1-5-21-2669793992-3689076831-3814312677-500"
                },
                ...
            ],
            "kill":[
                1111,2222,3333
            ]
        }
}
*/
#define DBG_EVENT_PROC_CHANGED       "ProcChanged"

//{"tid":12354}
#define DBG_EVENT_SYSTEM_BREAKPOINT  "SyatemBreakpoint"

/*
{
    "cmd":"event",
    "content":{
        "eventType":"UserBreakpoint",
        "mode":1,                                                       //1:չʾ��Ϣ��2:�����Ϣ
        "eventLabel":"Default",                                         //չʾ��ǩ
        "eventShow":""�����û��ϵ� 0xaabb1122 kernel32!CreateFileW\n"",   //չʾ����
        "eventResult": {
            "tid":1234,
            "addr":"0x1234abcd",
            "symbol":"kernel32!CreateFileW"
        }
}
*/
#define DBG_EVENT_USER_BREAKPOINT    "UserBreakpoint"
/*********************�����¼�����**********************************/
#endif //DBGPROTOCOL_DBGCTRL_H_H_