#ifndef BUFFER_H
#define BUFFER_H

#define SZ_BUF (1024*1024) // 1M

typedef struct {
    uint8_t *raw;
    size_t sz;
    size_t idx_read;
    size_t idx_write;
} buf_t;

void buf_create(buf_t **buf);

void buf_destory(buf_t *buf);

size_t buf_readjust(buf_t *buf, size_t sz);

size_t buf_realloc(buf_t *buf, size_t sz);

size_t write_buf(buf_t *buf, uint8_t *data, size_t sz);

size_t read_buf(buf_t *buf, uint8_t *data, size_t sz);

size_t peek_buf(buf_t *buf, uint8_t *data, size_t sz);

size_t buf_to_sock(buf_t *buf, apr_socket_t *sock);

size_t buf_from_sock(buf_t *buf, apr_socket_t *sock);

#endif BUFFER_H
