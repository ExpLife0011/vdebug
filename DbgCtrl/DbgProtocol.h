#ifndef DBGPROTOCOL_DBGCTRL_H_H_
#define DBGPROTOCOL_DBGCTRL_H_H_

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
/*
{
    "cmd":"RunCmd",
    "content":{
        "cmd":"bp kernen32!CreateFileW"
    }
}

{
    "cmd": "reply",
    "content": {
        "command": "kv",
        "status": 0,
        "reason": "abcdef",
        "result": [{
            "retaddr": "0x0xabcd12ff",
            "param0": "0xabcd1234",
            "param1": "0xabcd1234",
            "param2": "0xabcd1233"
        }]
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
#define DBG_TASK_GET_PROC            "GetProcInfo"


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
#define DBG_EVENT_PROC_CHANGEDA      "ProcChanged"

//{"tid":12354}
#define DBG_EVENT_SYSTEM_BREAKPOINT   "SyatemBreakpoint"
#define DBG_EVENT_SYSTEM_BREAKPOINTA  "SyatemBreakpoint"
#endif //DBGPROTOCOL_DBGCTRL_H_H_