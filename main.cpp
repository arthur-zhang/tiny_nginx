#include <iostream>
#include <unistd.h>
#include "include/ngx_c_conf.h"
#include "include/ngx_func.h"
#include "include/ngx_c_socket.h"
#include "include/global.h"
using namespace std;
pid_t ngx_pid;
pid_t ngx_ppid;
int ngx_process;
CSocket g_socket;

int main() {
    std::cout << "Hello, World!, parent pid=" << getpid() << std::endl;

    ngx_pid = getpid();
    ngx_ppid = getppid();
    auto *p_config = CConfig::GetInstance();
    if (!p_config->Load("./nginx.conf.toml")) {
        std::cout << "load config failed..." << std::endl;
        exit(1);
    }
    g_socket.Initialize();

    ngx_master_process_cycle();

    std::cout << "exit..." << std::endl;
    return 0;
}
