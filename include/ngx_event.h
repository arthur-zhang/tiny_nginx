#pragma once

struct ngx_event_s {
    void *data;
    unsigned instance: 1;
//    unsigned active: 1;
    unsigned ready: 1;
    ngx_event_handler_pt handler;
};
