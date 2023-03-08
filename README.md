# ICS2019 Programming Assignment

南京大学 计算机系统基础 课程实验 2019. [实验手册](https://nju-projectn.github.io/ics-pa-gitbook/ics2019/)

实验时间: 2020/07/20 - 2020/08/22

实验已经完成了，可能会有一些小的bug, 欢迎来一起讨论呀！

* PA1 - 最简单的计算机 (已完成)
  * task PA1.1: 实现单步执行, 打印寄存器状态, 扫描内存
  * task PA1.2: 实现算术表达式求值 
  * task PA1.3: 实现所有要求(监视点)
* PA2 -  冯诺依曼计算机系统 (已完成)
	* task PA2.1: 实现用c语言模拟x86汇编指令的执行
	* task PA2.2: 继续实现用c语言模拟执行更多的x86汇编指令
	* task PA2.2: 实现基本的输入输出
* PA3 - 批处理系统 (大部分已完成,基础设施(3)未完成)
	* task PA3.1: 实现自陷操作`_yield()`及其过程 
	* task PA3.2: 实现用户程序的加载和系统调用, 支撑TRM程序的运行
	* task PA3.3: 运行仙剑奇侠传并展示批处理系统
* PA4 - 分时多任务 (已完成)
	* task PA4.1: 实现基本的多道程序系统 
	* task PA4.2: 实现支持虚存管理的多道程序系统 
	* task PA4.3: 实现抢占式分时多任务系统

This project is the programming assignment of the class ICS(Introduction to Computer System) in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to http://nju-ics.gitbooks.io/ics2019-programming-assignment/content/

To initialize, run
```bash
bash init.sh
```

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Nexus-am](https://github.com/NJU-ProjectN/nexus-am)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)
