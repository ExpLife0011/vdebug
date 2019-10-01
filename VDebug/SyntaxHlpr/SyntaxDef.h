#pragma once

#define LABEL_DEFAULT       "Default"
#define LABEL_LOG_CONTENT   "LogContent"
#define LABEL_DBG_CONTENT   "DbgContent"

#define LABEL_CMD_SEND   "CmdSend"
#define LABEL_CMD_RECV   "CmdRecv"
#define LABEL_CMD_HIGHT  "cmdHight"

#define LABEL_CALLSTACK     "CallStack"
#define LABEL_TCP_PIPE1     "TcpPipe1"
#define LABEL_TCP_PIPE2     "TcpPipe2"

//测试Demo相关标签
#define LABEL_DEMO_TEST1     "DemoTest1"
#define LABEL_DEMO_TEST2     "DemoTest2"
#define LABEL_DEMO_TEST3     "DemoTest3"
#define LABEL_DEMO_TEST4     "DemoTest4"

/*
20190618
STYLE 用于设置字体的颜色等属性，需要注意,最大的STYLE值是256,
超过这个值会导致设置的颜色属性无效，STYLE_DEFAULT = 32
我们的范围从101开始设置
*/
#define STYLE_CONTENT           101
#define STYLE_FILTER            102
#define STYLE_SELECT            103
#define STYLE_ERROR             104
#define STYLE_TCP_PIPE1         105    //tcp流样式1
#define STYLE_TCP_PIPE2         106    //tcp流样式2

#define COLOUR_DEFAULT_TEXT     RGB(0x00, 0xff, 0x00)
#define COLOUR_DEFAULT_BACK     RGB(0x00, 0x00, 0x00)

//cmd命令样式
#define STYLE_CMD_DEFAULT       107    //cmd default样式
#define STYLE_CMD_SEND          108    //cmd send样式
#define STYLE_CMD_RECV          109    //cmd recv样式
#define STYLE_CMD_HIGHT         110    //cmd hight样式

//测试Demo相关样式
#define STYLE_DEMO_DEFAULT      119    //默认样式
#define STYLE_DEMO_CALL         120    //call指令
#define STYLE_DEMO_JMP          121    //jmp指令
#define STYLE_DEMO_HEX1         122    //hex addr
#define STYLE_DEMO_HEX2         123    //hex str
#define STYLE_DEMO_HEX3         124    //hex byte
#define STYLE_DEMO_PROC         125    //模块函数

#define STYLE_LOG_KEYWORD_BASE  160    //日志关键字
#define STYLE_LOG_WARN          251    //日志警告
#define STYLE_LOG_ERROR         252    //日志错误

#define NOTE_KEYWORD    SCE_UNIVERSAL_FOUND_STYLE_EXT1      //关键字高亮
#define NOTE_SELECT     SCE_UNIVERSAL_FOUND_STYLE_EXT2      //选择高亮

//vdebug相关标签定义
#define LABEL_DBG_DEFAULT       "DbgDefault"
#define LABEL_DBG_SEND          "DbgSend"
#define LABEL_DBG_RECV          "DbgRecv"
#define LABEL_DBG_CALLSTACK     "DbgCallStack"
#define LABEL_DBG_MODULE        "DbgModule"
#define LABEL_DBG_HEX1          "DbgHex1"
#define LABEL_DBG_HEX2          "DbgHex2"
#define LABEL_DBG_HEX3          "DbgHex3"

#define STYLE_DBG_SEND_DEFAULT  101
#define STYLE_DBG_RECV_DEFAULT  102

#define STYLE_DBG_DEFAULT     1
#define STYLE_DBG_NUMBER      2       //for number style
#define STYLE_DBG_FUNCTION    3       //for function msg
#define STYLE_DBG_ADDR        4
#define STYLE_DBG_REGISTER    5
#define STYLE_DBG_ERROR       6
#define STYLE_DBG_MESSAGE     7
#define STYLE_DBG_HEX         8
#define STYLE_DBG_DATA        9
#define STYLE_DBG_BYTE        10
#define STYLE_DBG_INST        11
#define STYLE_DBG_CALL        12
#define STYLE_DBG_JMP         13
#define STYLE_DBG_PROC        14
#define STYLE_DBG_MODULE      15
#define STYLE_DBG_PARAM       16
#define STYLE_DBG_KEYWORD     17
#define STYLE_DBG_HIGHT       18
