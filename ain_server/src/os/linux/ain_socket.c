#include "ain_config.h"

#define _AIN_SOCKET_FILE_
#include "ain_globals.h"

#include "os/ain_inet.h"
#include "os/ain_io.h"
#include "os/ain_socket.h"

#include "core/ain_conn.h"
#include "core/ain_event.h"
#include "core/ain_sock.h"
#include "core/ain_worker_proc.h"

// use reuseport

ain_result_t ain_socket_accept(ain_event_t *evt) {
  ain_conn_t *conn = *(ain_conn_t **)EVENT_DATA(evt);
  struct io_uring_sqe *sqe = io_uring_get_sqe(&evt->aio->ring);
  io_uring_prep_accept(sqe, conn->ls->fd, &conn->client_addr,
                       &conn->client_addrlen, 0);
  evt->handler = ain_socket_onaccept;
  io_uring_sqe_set_data(sqe, evt);
  io_uring_submit(&evt->aio->ring);
  return AIN_OK;
}

ain_result_t ain_socket_onaccept(ain_event_t *evt) {
  ain_conn_t *conn = *(ain_conn_t **)EVENT_DATA(evt);
  if (evt->error) {
    ain_conn_close(conn);
    return AIN_ERROR;
  }
  evt->fd = conn->fd = evt->res;
  conn->connected = 1;
  char addrstr[64];
  printf("%02hu [%d] ain_socket_onaccept from %s\n", conn->ls->wdata->id,
         evt->fd,
         ain_inet_ntop(conn->ls->sockaddr.sa_family, &conn->client_addr,
                       addrstr, sizeof(addrstr)));
  return ain_socket_read(evt);
}

static ain_result_t ain_socket_onread(ain_event_t *evt) {
  printf("[%d] ain_socket_onread %d\n", evt->fd, evt->res);
  evt->buf_size = evt->res;
  return ain_conn_onread(evt);
}

int ain_socket_read(ain_event_t *evt) {
  printf("[%d] ain_socket_read buf %zu x %u\n", evt->fd, evt->iov[0].iov_len,
         evt->iovcnt);
  struct io_uring_sqe *sqe = io_uring_get_sqe(&evt->aio->ring);
  evt->handler = ain_socket_onread;
  io_uring_prep_readv(sqe, evt->fd, evt->iov, evt->iovcnt, 0);
  io_uring_sqe_set_data(sqe, evt);
  io_uring_submit(&evt->aio->ring);
  printf("[%d] ain_socket_read submit\n", evt->fd);
  return AIN_OK;
}

static ain_result_t ain_socket_onwrite(ain_event_t *evt) {
  printf("[%d] ain_socket_onwrite %d\n", evt->fd, evt->res);
  evt->buf_size = evt->res;
  return ain_conn_onwrite(evt);
}

ain_result_t ain_socket_write(ain_event_t *evt) {
  printf("[%d] ain_socket_write buf %zu x %u\n", evt->fd, evt->buf_size,
         evt->iovcnt);
  struct io_uring_sqe *sqe = io_uring_get_sqe(&evt->aio->ring);
  evt->handler = ain_socket_onwrite;
  io_uring_prep_writev(sqe, evt->fd, evt->iov, evt->iovcnt, 0);
  io_uring_sqe_set_data(sqe, evt);
  io_uring_submit(&evt->aio->ring);
  printf("[%d] ain_socket_write submit\n", evt->fd);
  return AIN_OK;
}