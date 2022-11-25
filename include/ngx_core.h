#pragma once

//#include "ngx_connection.h"
//#include "ngx_event.h"
#include <functional>

struct ngx_connection_s;
struct ngx_event_s;
struct ngx_listening_s;

typedef ngx_connection_s ngx_connection_t;
typedef ngx_event_s ngx_event_t;
typedef std::function<void(ngx_event_t *)> ngx_event_handler_pt;


typedef struct ngx_listening_s ngx_listening_t;

