#ifndef DBGPROTOCOL_DBGCTRL_H_H_
#define DBGPROTOCOL_DBGCTRL_H_H_

//��Ϣ�����ctrl �� event����
//ctrl �ǵ��Կ��ƶ˿��Ƶ�����ִ�е��Զ���
//event �ǵ���������Կ��ƶ��ϱ������¼�

/*********************���Կ���ָ�ʼ*****************************/
/*
{
    "cmd":"attach",
    "content":{
        "pid":1234
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

//�жϵ�����
#define DBG_CTRL_BREAK               "BreakDebugger"
/*********************���Կ���ָ�����******************************/

/*********************�����¼���ʼ**********************************/
/*
{
    "cmd":"event",
    "content":{
        "type":"DbgMessage",
        "data":{
            "abcdef"
        }
    }
}
*/
#define DBG_DBG_EVENT                   "event"

//Debug Event
/*
{
    "cmd":"event",
    "content":{
        "type":"DbgMessage",
        "data":{
            "abcdef"
        }
    }
}
*/
#define DBG_EVENT_MSG                    "dbgmsg"
/*
{
    "cmd":"event",
    "content":{
        "type":"proccreate",
        "data":{
            "pid":1234,
            "image":"d:\\desktop\\1234.exe",
            "baseAddr":"0x4344353",
            "entryAddr":"0x4344389"
        }
    }
}
*/
#define DBG_EVENT_DBG_PROC_CREATE        "proccreate"
#define DBG_EVENT_DBG_PROC_END           "procend"

#define DBG_EVENT_DBG_PROC_CREATEA       "proccreate"
#define DBG_EVENT_DBG_PROC_ENDA          "procend"

/*
{
    "cmd":"event",
    "content":{
        "type":"procRunning",
        "data":{
            "abcdef"
        }
    }
}
*/
#define DBG_EVENT_DBG_PROC_RUNNING       "procRunning"          //���Խ���ִ����

/*
{
    "cmd":"event",
    "content":{
        "type":"moduleload",
        "data":{
            "name":"kernel32.dll",
            "baseAddr":"0x4344353",
            "endAddr":"0x43443ff"
        }
    }
}
*/
#define DBG_EVENT_MODULE_LOAD        "moduleload"
#define DBG_EVENT_MODULE_UNLOAD      "moduelunload"
#define DBG_EVENT_MODULE_LOADA       "moduleload"
#define DBG_EVENT_MODULE_UNLOADA     "moduelunload"

//{}
#define DBG_EVENT_DETACH             "eventDetach"


/*
{
    "cmd":"event",
    "content":{
        "type":"proc_add",
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
*/
#define DBG_EVENT_PROC_CHANGED       "ProcChanged"

//{"tid":12354}
#define DBG_EVENT_SYSTEM_BREAKPOINT   "SyatemBreakpoint"
/*********************�����¼�����**********************************/
#endif //DBGPROTOCOL_DBGCTRL_H_H_