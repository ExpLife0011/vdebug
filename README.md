# vdebug
#### 开发记录
之前一直用的微软自家的windbg调试器，有些功能用的不大习惯，比如windbg的条件断点，语法畸形并且效率很低，于是就有了自己开发一个的念头，正好开发的过程中自己也学习调试方面的相关知识，于是就有了这个项目，然后业余时间做了很久，后来由于转android开发了，一直搁置，部分功能尚未完成，自己也没有很多时间开发，于是就想将这个项目开源，让大家一起来完善这个项目。欢迎调试大牛来帮忙完善这个调试器，新人也可以通过这个项目来学习调试的相关知识。

#### 功能介绍
vdebug是一个位于windows操作系统的三环调试器，支持大部分的调试功能，比如函数断点，单步调试，读写断点，反汇编，查看内存数据，查看函数调用栈，关联微软调试符号，条件断点等。此外，该项目也有自己的一些特色功能，比如录入函数模型在函数断点断下后自动分析函数的参数信息，分析dump内存影像文件等等。
主界面截图：
![输入图片说明](https://images.gitee.com/uploads/images/2019/1005/115408_46baa519_498054.png "111.png")
![输入图片说明](https://images.gitee.com/uploads/images/2019/1005/115433_27b265bf_498054.png "222.png")

#### 软件架构
```
├─ComLib     项目公共动态库，实现各个项目通用的功能。
├─dbg        调试器核心功能模块，实现调试器的核心功能
├─DbgCtrl    调试功能代理中间件，调试控制层和调试实现层的中间件
├─mq         消息中间件，用于不同的进程，不同的组件之间实时通讯
├─runner     功能启动器，用于通过服务启动调试器的各个组件来提升调试组件的权限
└─vdebug     调试器的控制层和展示层，用于数据展示以及与dbg进行调试交互
```

#### 使用说明

1. xxxx
2. xxxx
3. xxxx

#### 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request


#### 码云特技

1. 使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2. 码云官方博客 [blog.gitee.com](https://blog.gitee.com)
3. 你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解码云上的优秀开源项目
4. [GVP](https://gitee.com/gvp) 全称是码云最有价值开源项目，是码云综合评定出的优秀开源项目
5. 码云官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6. 码云封面人物是一档用来展示码云会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)