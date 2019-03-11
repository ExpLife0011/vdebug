#ifndef DBGPROTOCOL_DBGCTRL_H_H_
#define DBGPROTOCOL_DBGCTRL_H_H_

/*
协议按应用场景抽象成两类，一问一答的应答方式和单向的数据上报
一问一答的应答方式
请求方：
{
    "type":"ctrl",
    "cmd":"attach",
    "content": {
        "pid":1234
    }
}

应答方：
{
    "status":0,
    "label":"default",
    "show":"showmsg",
    "result":{
        ...
    }
}

单向数据推送
{
    "type":"event",
    "event":"moduleload",
    "label":"default",
    "show":"xxxx 模块加载",
    "content":{
    }
}

调试命令作为ctrl的子类
{
    "type":"ctrl",
    "cmd":"RunCmd",
    "content": {
        "command":"kv"
    }
}
*/
//附加方式调试
#define DBG_CTRL_ATTACH              "attach"
//执行可执行文件
#define DBG_CTRL_EXEC                "exec"
//脱离调试器
#define DBG_CTRL_DETACH              "ctrlDetach"
//执行调试命令
#define DBG_CTRL_RUNCMD              "RunCmd"
//调试控制端获取进程信息
#define DBG_CTRL_GET_PROC            "GetProcInfo"
//分析dump文件
#define DBG_CTRL_OPEN_DUMP           "OpenDump"
//为进程生成dump文件
#define DBG_CTRL_DUMP_PROC           "DumpProc"
//中断调试器
#define DBG_CTRL_BREAK               "BreakDebugger"

//函数类型录入
#define DBG_CTRL_TEST_DESC           "DescTest"
#define DBG_CTRL_INPUT_DESC          "DescSave"
/*********************调试控制指令结束******************************/

/*********************调试事件开始**********************************/
#define DBG_DBG_EVENT                "event"
//调试信息事件
#define DBG_EVENT_MSG                "dbgmsg"
//进程创建事件
#define DBG_EVENT_DBG_PROC_CREATE    "proccreate"
//进程退出事件
#define DBG_EVENT_DBG_PROC_END       "procend"
//调试进程执行事件
#define DBG_EVENT_DBG_PROC_RUNNING   "procRunning"
//模块加载事件
#define DBG_EVENT_MODULE_LOAD        "moduleload"
//模块卸载事件
#define DBG_EVENT_MODULE_UNLOAD      "moduelunload"
//停止调试事件
#define DBG_EVENT_DETACH             "eventDetach"
//异常事件分发
#define DBG_EVENT_EXCEPTION          "exception"
//进程变化事件
#define DBG_EVENT_PROC_CHANGED       "ProcChanged"
//系统中断事件
#define DBG_EVENT_SYSTEM_BREAKPOINT  "SyatemBreakpoint"
//用户断点事件
#define DBG_EVENT_USER_BREAKPOINT    "UserBreakpoint"
/*********************调试事件结束**********************************/
#endif //DBGPROTOCOL_DBGCTRL_H_H_