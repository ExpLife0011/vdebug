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
    "cmd":"kv",
    "content":{
        "param":"abcdef"
    }
}

{
    "cmd":"reply",
    "content":{
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
*/
#define DBG_CTRL_KV                 L"kv"
#endif //DBGPROTOCOL_DBGCTRL_H_H_