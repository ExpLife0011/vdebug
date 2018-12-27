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
#define DBG_DBG_EVENT               L"event"

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
#define DBG_EVENT_MSG               L"dbgmsg"
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
#define DBG_EVENT_PROC_CREATE       L"proccreate"
#define DBG_EVENT_PROC_END          L"procend"
#define DBG_EVENT_MODULE_LOAD       L"moduleload"
#define DBG_EVENT_MODULE_UNLOAD     L"moduelunload"

/*
{
    "cmd":"attach",
    "content":{
        "pid":1234
    }
}
*/
#define DBG_CTRL_ATTACH             L"attach"

/*
{
    "cmd":"exec",
    "content":{
        "path":"abcdef.exe",
        "param":"abcdefgh"
    }
}
*/
#define DBG_CTRL_EXEC               L"exec"
/*
{
    "cmd":"RunCmd",
    "content":{
        "command":"bp kernen32!CreateFileW"
    }
}

{
    "cmd":"reply",
    "content":{
        "command":"kv",
        "status":0,
        "reason":"abcdef",
        "result":{
            [
                {
                    "retaddr":0x0xabcd12ff,
                    "param0":0xabcd1234,
                    "param1":0xabcd1234,
                    "param2":0xabcd1233
                },
                ...
            ]
        }
    }
}
*/
#define DBG_CTRL_RUNCMD              L"RunCmd"
#endif //DBGPROTOCOL_DBGCTRL_H_H_