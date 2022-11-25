//
// Created by arthur on 2022/10/27.
//

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../include/ngx_core.h"
#include "../include/ngx_c_conf.h"
#include "../include/global.h"
#include "../include/ngx_connection.h"
#include <iostream>

#ifdef __APPLE__

#include "../epoll/epoll.h"

#elif __linux
#include <sys/epoll.h>
#endif

#include <functional>

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;

void CSocket::ReadConf() {
    auto p_config = CConfig::GetInstance();
    m_ListenPorts = p_config->GetIntArray("ListenPorts");
    m_WorkerConnections = p_config->GetIntDefault("WorkerConnections", 0);
    m_ListenPortCount = m_ListenPorts.size();
    std::cout << "listen port count: " << m_ListenPortCount << std::endl;
}

ngx_connection_t *CSocket::ngx_get_connection(int sockfd) {
    ngx_connection_t *c = free_connections;
    if (c == nullptr) {
        std::cout << "free connection list empty" << std::endl;
        return nullptr;
    }
    free_connections = static_cast<ngx_connection_t *>(c->data);
    free_connection_n--;

    ngx_event_t *rev = c->read;
    ngx_event_t *wev = c->write;
    memset(c, 0, sizeof(ngx_connection_t));

    c->read = rev;
    c->write = wev;
    c->fd = sockfd;
    rev->data = c;
    wev->data = c;

    return c;
}

int CSocket::ngx_epoll_init() {
    m_Epfd = epoll_create(1);

    connection_n = m_WorkerConnections;
    connections = new ngx_connection_t[connection_n];
    read_events = new ngx_event_t[connection_n];
    write_events = new ngx_event_t[connection_n];
    ngx_connection_t *c = connections;
    ngx_connection_t *next = nullptr;
    int i = connection_n;
    do {
        i--;
        c[i].data = next;
        c[i].read = &read_events[i];
        c[i].write = &write_events[i];
        c[i].fd = -1;
        next = &c[i];
    } while (i > 0);
    free_connections = next;
    free_connection_n = connection_n;


    for (const auto &item: m_ListenSocketList) {
        c = ngx_get_connection(item->fd);
        if (c == nullptr) {
            std::cout << "should not happen" << std::endl;
            exit(1);
        }
        c->listening = item;
        c->listening->port = item->port;
        item->connection = c;
        c->read->handler = std::bind(&CSocket::ngx_event_accept, this, _1);
        ngx_epoll_add_connection(c);
    }
    return 1;
}

void CSocket::ngx_read_request_handler(ngx_event_t *e) {
    ngx_connection_t *c = (ngx_connection_t *) e->data;

    std::cout << "ngx_read_request_handler" << std::endl;
    char buf[512];
    ssize_t n = recv(c->fd, buf, sizeof(buf), 0);

    if (n == 0) return;

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        if (errno == EINTR) {
            return;
        }
        if (errno == ECONNRESET) {
            return;
        }
    }
    std::cout << "recv data: " << buf << std::endl;
    c->write->ready = 1;
}

void CSocket::ngx_write_request_handler(ngx_event_t *e) {
    if (!e->ready) {
        return;
    }
    ngx_connection_t *c = (ngx_connection_t *) e->data;

    std::cout << "ngx_write_request_handler" << std::endl;
    char buf[512];
    buf[0] = 'h';
    buf[1] = 'e';
    buf[2] = 'l';
    buf[3] = 'l';
    buf[4] = 'o';
    write(c->fd, buf, 5);
    c->write->ready = 0;
}

void CSocket::ngx_event_accept(ngx_event_t *c) {
    ngx_connection_t *lc = (ngx_connection_t *) c->data;
    std::cout << "[" << ngx_pid << "]" << "event accept" << ",fd=" << lc->fd << ",port=" << lc->listening->port
              << std::endl;
    int fd = lc->fd;
    struct sockaddr addr;
    socklen_t socklen = sizeof(addr);
    int sockfd = accept(fd, &addr, &socklen);
    int err;
    if (sockfd < 0) {
        err = errno;
        std::cout << strerror(err) << std::endl;
        if (err == EAGAIN) {
            return;
        }
    }
    std::cout << "accept done: " << sockfd << std::endl;
    ngx_connection_t *newc = ngx_get_connection(sockfd);
    if (newc == nullptr) {
        std::cout << "connection pool full" << std::endl;
        close(sockfd);
        return;
    }
    setnonblocking(sockfd);
    newc->listening = lc->listening;
    newc->read->handler = std::bind(&CSocket::ngx_read_request_handler, this, _1);
    newc->write->handler = std::bind(&CSocket::ngx_write_request_handler, this, _1);

    ngx_epoll_add_connection(newc);
    return;
}

int CSocket::ngx_epoll_add_connection(ngx_connection_t *c) {
    std::cout << "ngx_epoll_add_connection..." << std::endl;
    struct epoll_event ee;
    ee.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
    ee.data.ptr = (void *) ((uintptr_t) c | c->instance);
    if (epoll_ctl(m_Epfd, EPOLL_CTL_ADD, c->fd, &ee) < 0) {
        return NGX_ERROR;
    }
    return NGX_OK;
}

int CSocket::ngx_epoll_add_event(int readevent, int writeevent, uint32_t otherflag,
                                 uint32_t eventtype, ngx_connection_t *c) {
    int op;
    struct epoll_event event;
    if (readevent) {
        event.events |= EPOLLIN | EPOLLRDHUP;
    }
    event.data.ptr = (void *) ((uintptr_t) c | c->instance);
    int rc = epoll_ctl(m_Epfd, eventtype, c->fd, &event);
    if (rc < 0) {
        return -1;
    }
    return 1;
}

int CSocket::ngx_epoll_process_events(int timeout) {

    int events;
    uint32_t revents;
    ngx_connection_t *c;

    std::cout << "ngx_epoll_process_events, timeout=" << timeout << std::endl;
    events = epoll_wait(m_Epfd, m_events, NGX_MAX_EVENTS, timeout);
    if (events < 0) {
        if (errno == EINTR) {
            return NGX_ERROR;
        } else {
            return NGX_OK;
        }
    }
    if (events == 0) {
        if (timeout != NGX_TIMER_INFINITE) {
            return NGX_OK;
        }
        return NGX_ERROR;
    }
    std::cout << "epoll_wait return, events:" << events << std::endl;
    for (int i = 0; i < events; ++i) {
        void *p = m_events[i].data.ptr;
        uintptr_t instance = (uintptr_t) p & 1;
        c = (ngx_connection_t *) ((uintptr_t) p & (uintptr_t) ~1);
        if (c->fd == -1 || c->instance != instance) {
            continue;
        }
        revents = m_events[i].events;
        if (revents & (EPOLLERR | EPOLLHUP)) {
            std::cout << "EPOLLERR | EPOLLHUP" << std::endl;
            revents |= EPOLLIN | EPOLLOUT;
        }
        if (revents & EPOLLIN) {
            std::cout << "EPOLLIN......" << std::endl;
//            c->read->handler(this, c->read);
            c->read->handler(c->read);
        }
        if (revents & EPOLLOUT) {
            std::cout << "EPOLLOUT......" << std::endl;
            c->write->handler(c->write);
        }
    }
    return NGX_OK;
}

bool CSocket::setnonblocking(int fd) {
    int nb = 1;
    if (ioctl(fd, FIONBIO, &nb) < 0) return false;
    return true;
}

bool CSocket::ngx_open_listening_sockets() {
    auto p_config = CConfig::GetInstance();
    vector<int> ports = p_config->GetIntArray("ListenPorts");
    int sockfd;
    struct sockaddr_in serv_adr;
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    for (int i = 0; i < m_ListenPortCount; ++i) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return false; // todo
        }
        int reuseaddr = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &reuseaddr, sizeof(reuseaddr));
        if (!setnonblocking(sockfd)) return false;

        serv_adr.sin_port = htons(ports[i]);
        if (::bind(sockfd, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0) {
            ::close(sockfd);
            return false; // todo
        }
        if (listen(sockfd, NGX_LISTEN_BACKLOG) < 0) {
            ::close(sockfd);
            return false; // todo
        }

        auto p_listensockitem = new ngx_listening_t;
        p_listensockitem->fd = sockfd;
        p_listensockitem->port = ports[i];
        m_ListenSocketList.emplace_back(p_listensockitem);
    }
    return m_ListenSocketList.size() > 0;
}

bool CSocket::Initialize() {
    ReadConf();
    ngx_open_listening_sockets();
    return false;
}
