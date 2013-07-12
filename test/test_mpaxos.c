/*
 * test_mpaxos.cc
 *
 *  Created on: Jan 29, 2013
 *      Author: ms
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>
#include <json/json.h>
#include <apr_time.h>
#include <pthread.h>
#include <inttypes.h>
#include <apr_thread_proc.h>
#include <apr_thread_pool.h>

#include "mpaxos/mpaxos.h"
#include "mpaxos/mpaxos-config.h"

#define MAX_THREADS 100

//This macro should work for gcc version before 4.4
#define __STDC_FORMAT_MACROS
// do not include from ../libmpaxos/utils/logger.h
// it sucks, we should not include from internal library
// if you really want to do that, put the logger into the include/mpaxos directory or this directory as some header lib
// besides, that logger sucks too.
#define LOG_INFO(x, ...) printf("[%s:%d] [ II ] "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)

uint32_t n_tosend = 1000;
int n_group = 1;
int async = 0;
int run = 1;
static int is_exit_ = 0;
static int t_sleep_ = 2;

static apr_pool_t *pl_test_;
static apr_thread_pool_t *tp_test_;



void cb(groupid_t gid, slotid_t sid, uint8_t *data, size_t sz_data, void* para) {
    LOG_INFO("asynchronous callback called. gid:%d, sid:%d", gid, sid);
}


void* APR_THREAD_FUNC test_group(apr_thread_t *p_t, void *v) {
    groupid_t gid = (groupid_t)(uintptr_t)v;
    apr_time_t start_time;
    apr_time_t last_time;
    apr_time_t curr_time;

    uint64_t msg_count = 0;
    uint64_t data_count = 0;
    int val_size = 64;
    uint8_t *val = (uint8_t *) calloc(val_size, sizeof(char));

    start_time = apr_time_now();
    last_time = apr_time_now();

    //Keep sending

    int n = n_tosend;
    do {
        if (async) {
            commit_async(&gid, 1, val, val_size, NULL);
        } else {
            commit_sync(&gid, 1, val, val_size);
        }
        msg_count++;
        data_count += val_size;
    } while (--n > 0);
    curr_time = apr_time_now();
    double period = (curr_time - start_time) / 1000000.0;
    double prop_rate = (msg_count + 0.0) / period;
    LOG_INFO("gid:%u, %"PRIu64" proposals commited in %.2fs, rate:%.2f props/s",
            gid, msg_count, period, prop_rate);
    LOG_INFO("%.2f MB data sent, speed:%.2fMB/s",
            (data_count + 0.0) / (1024 * 1024),
            (data_count + 0.0) / (1024 * 1024) /period);
    free(val);
    return NULL;
}

void init_thread_pool() {
    apr_initialize();
    apr_pool_create(&pl_test_, NULL);
    apr_thread_pool_create(&tp_test_, MAX_THREADS, MAX_THREADS, pl_test_);
    
}

void destroy() {
    apr_pool_destroy(pl_test_);
}


int main(int argc, char **argv) {
    
    int c = 1;
    // initialize from config
    char *config = NULL;
    if (argc > c) {
        config = argv[c];
    } else {
        printf("Usage: %s config [run=1] [n_tosend=1000] [n_group=1] [async=0] [is_exit_=0] [sleep_time=2]\n", argv[0]);
        exit(0);
    }
    
    init_thread_pool();
    
    // init mpaxos
    mpaxos_init();
    
    int ret = mpaxos_load_config(config);
    if (ret != 0) {
        mpaxos_destroy();
        exit(10);
    }
    
    run = (argc > ++c) ? atoi(argv[c]) : run;
    n_tosend = (argc > ++c) ? atoi(argv[c]) : n_tosend;
    n_group = (argc > ++c) ? atoi(argv[c]) : n_group;
    async = (argc > ++c) ? atoi(argv[c]) : async;
    is_exit_ = (argc > ++c) ? atoi(argv[c]) : is_exit_;
    t_sleep_ = (argc > ++c) ? atoi(argv[c]) : t_sleep_;
    
    LOG_INFO("test for %d messages.", n_tosend);
    LOG_INFO("test for %d threads.", n_group);
    LOG_INFO("async mode %d", async);

    // start mpaxos service
    mpaxos_set_cb_god(cb);
    mpaxos_start();

    if (run) {
        // wait some time to wait for everyone starts
        sleep(t_sleep_);

        LOG_INFO("serve as a master.");
        apr_time_t begin_time = apr_time_now();

        for (uint32_t i = 0; i < n_group; i++) {
            apr_thread_pool_push(tp_test_, test_group, (void*)(uintptr_t)(i+1), 0, NULL);
        }
        while (apr_thread_pool_tasks_count(tp_test_) > 0) {
            sleep(0);
        }
        
        apr_thread_pool_destroy(tp_test_);
        apr_time_t end_time = apr_time_now();
        double time_period = (end_time - begin_time) / 1000000.0;
        uint64_t n_msg = n_tosend * n_group;
        LOG_INFO("in total %"PRIu64" proposals commited in %.2fsec," 
                "rate:%.2f props per sec.", n_msg, time_period, n_msg / time_period); 
    }

    if (is_exit_) {
        apr_sleep(2000000);
        LOG_INFO("Goodbye! I'm about to destroy myself.");
        mpaxos_destroy();
        exit(0);
    }

    LOG_INFO("All my task is done. Go on to serve others.");

    // I want to bring all the evil with me before I go to hell.
    // cannot quit because have to serve for others
    while (true) {
        fflush(stdout);
        apr_sleep(1000000);
    }
    
    destroy();
}

