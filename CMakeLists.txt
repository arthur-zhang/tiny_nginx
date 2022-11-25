cmake_minimum_required(VERSION 3.19)
project(tiny-nginx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
if (MACOS)
    include_directories(epoll)
endif ()
include_directories(deps/toml)
add_executable(tiny_nginx main.cpp app/ngx_c_conf.cpp include/ngx_c_conf.h
        include/ngx_func.cpp include/ngx_func.h proc/ngx_process_cycle.cpp include/global.h net/ngx_c_socket.cpp include/ngx_c_socket.h net/ngx_epoll_module.cpp include/ngx_epoll_module.h include/ngx_event.h include/ngx_core.h include/ngx_connection.h)
