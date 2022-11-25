#pragma once


#include "ngx_core.h"

struct ngx_listening_s {
    int port;
    int fd;
    ngx_connection_t *connection;
};

struct ngx_connection_s {
    void *data;
    int fd;
    ngx_listening_t *listening; // pointer to ngx_listening_t
    unsigned instance: 1;
    struct sockaddr *sockaddr; // client sockaddr
    struct sockaddr *local_sockaddr; // client sockaddr
    ngx_event_t *read;
    ngx_event_t *write;
};
