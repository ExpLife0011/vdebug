#ifndef DBGPROTOCOL_DBGCTRL_H_H_
#define DBGPROTOCOL_DBGCTRL_H_H_

/*
{
    "cmd":"DbgMessage_c2s",
    "content":{
        "msg":"abcdef"
    }
}
*/
#define DBG_EVENT_DBGMESSAGE        L"DbgMessage_c2s"

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