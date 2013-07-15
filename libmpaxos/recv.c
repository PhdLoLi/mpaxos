#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <event.h>
#include <event2/thread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <apr_thread_proc.h>
#include <apr_network_io.h>
#include <errno.h>
#include "recv.h"
#include "utils/logger.h"

#define MAX_THREADS 100

extern apr_pool_t *pl_global_;
apr_thread_t *t;
struct event_base *ev_base_;
struct event ev_listener_;
static int exit_ = 0;

void init_recvr(recvr_t* r) {
    apr_status_t status;
    apr_pool_create(&r->pl_recv, NULL);
    apr_sockaddr_info_get(&r->sa, NULL, APR_INET, r->port, 0, r->pl_recv);
    apr_thread_pool_create(&r->tp_recv, MAX_THREADS, MAX_THREADS, r->pl_recv);
    apr_socket_create(&r->s, r->sa->family, SOCK_DGRAM, APR_PROTO_UDP, r->pl_recv);
/*
    apr_socket_create(&r->s, r->sa->family, SOCK_SEQPACKET, APR_PROTO_SCTP, r->pl_recv);
*/
    apr_socket_opt_set(r->s, APR_SO_NONBLOCK, 0);
    apr_socket_timeout_set(r->s, -1);
    apr_socket_opt_set(r->s, APR_SO_REUSEADDR, 1);/* this is useful for a server(socket listening) process */

    status = apr_socket_bind(r->s, r->sa);
    if (status != APR_SUCCESS) {
        LOG_ERROR("cannot bind.");
        exit(0);
    }
    r->buf = (uint8_t*) apr_palloc(r->pl_recv, BUF_SIZE__);
/*
    int recv_buf_size = BUF_SIZE__;
    socklen_t optlen = sizeof(recv_buf_size);
    setsockopt(r->fd, SOL_SOCKET, SO_SNDBUF, &recv_buf_size, optlen);
*/
}

void on_read(void *arg) {
    recvr_t *r = (recvr_t*)arg;
    apr_status_t status;
    apr_size_t n = BUF_SIZE__;

//    LOG_DEBUG("start reading socket");
    apr_sockaddr_t remote_sa;
    status = apr_socket_recvfrom(&remote_sa, r->s, 0, (char *)r->buf, &n);
//    LOG_DEBUG("finish reading socket.");
    if (n < 0) {
        LOG_WARN("error when receiving message:%s", strerror(errno));
    } else if (n == 0) {
        LOG_WARN("received an empty message.");
    } else {
        struct read_state *state = malloc(sizeof(struct read_state));
        state->sz_data = n;
        state->data = malloc(n);
        memcpy(state->data, r->buf, n);
        state->recvr = r;
        
//        apr_thread_t *t;
//        apr_thread_create(&t, NULL, r->on_recv, (void*)state, r->pl_recv);
//        apr_thread_pool_push(r->tp_recv, r->on_recv, (void*)state, 0, NULL);
        (*(r->on_recv))(NULL, state);
        
//        size_t res_sz = 0;      
//        char *res_buf = NULL;
//        
//        (*(r->on_recv))(r->msg, n, &res_buf, &res_sz);
//        if (res_sz > 0) {
//            sendto(r->fd, res_buf, res_sz, 0, (struct sockaddr *)&cliaddr, len);
//            free(res_buf);
//        }
    }
//  printf("Event id :%d.\n", event);
}

void* APR_THREAD_FUNC run_recvr(apr_thread_t *t, void* arg) {
    recvr_t* r = arg;
//    evthread_use_pthreads();
//    ev_base_ = event_base_new();
//    event_set(&ev_listener_, r->fd, EV_READ|EV_PERSIST, on_accept, (void*)r);
//    event_base_set(ev_base_, &ev_listener_);
//    event_add(&ev_listener_, NULL);
//    event_base_dispatch(ev_base_);
//    LOG_DEBUG("event loop ends.");
//    return NULL;
    while (!exit_) {
        on_read(r);
    }
    apr_thread_exit(t, APR_SUCCESS);
    return NULL;
}

void stop_server() {
    if (t) {
        exit_ = 1;
        LOG_DEBUG("recv server ends.");
//        apr_thread_join(NULL, t);
    }
}

void run_recvr_pt(recvr_t* r) {
    apr_thread_create(&t, NULL, run_recvr, (void*)r, pl_global_);
}
