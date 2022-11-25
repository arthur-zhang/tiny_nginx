#pragma once

#include <csignal>
#include "../include/ngx_c_socket.h"
#define NGX_PROCESS_MASTER     0  //master进程，管理进程
#define NGX_PROCESS_WORKER     1  //worker进程，工作进程
#define NGX_LISTEN_BACKLOG  511   //已完成连接队列，nginx给511，我们也先按照这个来：不懂这个数字的同学参考第五章第四节


#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6
#define NGX_TIMER_INFINITE  -1


extern pid_t ngx_pid;
extern pid_t ngx_ppid;
extern int ngx_process;
extern CSocket g_socket;