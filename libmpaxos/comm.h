#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <stdbool.h>
#include "rpc/rpc.h"
#include "mpaxos/mpaxos.h"
#include "mpaxos/mpaxos-types.h"
#include "internal_types.h"

#define RPC_PREPARE     1
#define RPC_PROMISE     2
#define RPC_ACCEPT      3
#define RPC_ACCEPTED    4
#define RPC_LEARN       5
#define RPC_LEARNED     6
#define RPC_DECIDE      7

void comm_init();

void comm_destroy();

void set_local_nid(nodeid_t nid);

nodeid_t get_local_nid();

bool is_in_group(groupid_t gid);

void set_nid_sender(nodeid_t nid, const char* addr, int port);

void add_group(groupid_t);

void add_group_sender(groupid_t, nodeid_t);

void add_sender(uint32_t nid, const char *remote_ip,
      uint16_t remote_port);

void add_recvr(void* on_recv);

void send_to(nodeid_t nid, msg_type_t, const uint8_t *, size_t sz);

void send_to_group(groupid_t gid, msg_type_t, const uint8_t *buf, size_t sz);

void send_to_groups(groupid_t* gids, size_t sz_gids,
          msg_type_t, const char *buf, size_t sz);

void start_server(int port);

void connect_all_senders();

slotid_t send_to_slot_mgr(groupid_t, nodeid_t, uint8_t *, size_t);

#endif
