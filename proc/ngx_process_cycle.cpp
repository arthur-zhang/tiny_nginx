//
// Created by arthur on 2022/10/26.
//
#include <unistd.h>
#include "../include/ngx_c_conf.h"
#include "../include/global.h"
#include <iostream>

static void ngx_start_worker_processes(int);

void ngx_worker_process_cycle(int idx, string procName);

void ngx_worker_process_init(int);

static int ngx_spawn_process(int idx, string name);

void ngx_master_process_cycle() {
    int workProcess = CConfig::GetInstance()->GetIntDefault("WorkerProcesses", 1); //从配置文件中得到要创建的worker进程数量

    ngx_start_worker_processes(workProcess);

    for (;;) {
        sleep(2);
//        std::cout << "ngx_master_process_cycle..." << std::endl;
    }
}

void ngx_process_events_and_timers() {
    g_socket.ngx_epoll_process_events(-1);
}

void ngx_worker_process_cycle(int idx, string procName) {
    ngx_process = NGX_PROCESS_WORKER;
    std::cout << "enter worker_process_cycle: " << idx << " " << procName << std::endl;
    ngx_worker_process_init(idx);

    for (;;) {
//        sleep(2);
//        std::cout << "ngx_worker_process_cycle..." << std::endl;
        ngx_process_events_and_timers();
    }
}


static void ngx_start_worker_processes(int processNums) {
    for (int i = 0; i < processNums; ++i) {
        ngx_spawn_process(i, "worker process");
    }
}

static int ngx_spawn_process(int idx, string name) {
    pid_t pid;
    pid = fork();
    switch (pid) {
        case -1:
            return -1;
        case 0:
            ngx_ppid = ngx_pid;
            ngx_pid = getpid();
            ngx_worker_process_cycle(idx, "worker process");
            break;
        default:

            break;
    }
}

void ngx_worker_process_init(int idx) {
    g_socket.ngx_epoll_init();
}
