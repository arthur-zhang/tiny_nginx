#pragma once

#include <vector>

#ifdef __APPLE__

#include "../epoll/epoll.h"

#elif __linux
#include <sys/epoll.h>
#endif
#define NGX_MAX_EVENTS      512

#include "ngx_core.h"
#include "ngx_event.h"

class CSocket {
public:
    CSocket() : m_ListenPortCount(0) {}

    virtual ~CSocket() {}

    bool Initialize();

    void ngx_event_accept(ngx_event_t *);

    void ngx_read_request_handler(ngx_event_t *c);

    void ngx_write_request_handler(ngx_event_t *c);

    int ngx_epoll_init();

    int ngx_epoll_add_event(int readevent, int writeevent, uint32_t otherflag,
                            uint32_t eventtype, ngx_connection_t *c);

    int ngx_epoll_add_connection(ngx_connection_t *c);

    int ngx_epoll_process_events(int xx);

    ngx_connection_t *ngx_get_connection(int);

private:
    void ReadConf();

private:

    int m_Epfd;
    struct epoll_event m_events[NGX_MAX_EVENTS];
    int m_ListenPortCount;
    int m_WorkerConnections;
    std::vector<int> m_ListenPorts;
//    int m_worker_connections;

    std::vector<ngx_listening_t *> m_ListenSocketList;

    int connection_n;
    ngx_connection_t *connections;
    ngx_event_t *read_events;
    ngx_event_t *write_events;
    ngx_connection_t *free_connections;
    int free_connection_n;

    bool setnonblocking(int fd);

    bool ngx_open_listening_sockets();

};


