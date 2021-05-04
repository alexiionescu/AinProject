#include "ain_config.h"

#define _AIN_USE_SOCKETS_
#include "ain_globals.h"
#include "os/ain_memory.h"
#include "os/ain_socket.h"
#include "os/ain_io.h"
#include "core/ain_conn.h"
#include "core/ain_sock.h"
#include "core/ain_event.h"
#include "core/ain_worker_proc.h"
#include "core/ain_protocol.h"



ain_result_t ain_conn_init(ain_conn_t* conn, ain_sock_t* ls)
{
	conn->fd = (ain_fd_t)-1;
	conn->ls = ls;
	conn->proto.proto_handler = ain_proto_get_type(&ls->uri);
	return AIN_OK;
}

ain_result_t ain_conn_accept(ain_conn_t* conn)
{
	if(!conn->read_evt)
		conn->read_evt = ain_pool_alloc(conn->ls->wdata->pool, 1);

	conn->fd = ain_socket_create(conn->ls->sockaddr.sa_family, conn->ls->socktype, 0);
	AIN_ASSERT_ERR(conn->fd != (ain_fd_t)-1)
	conn->read_evt->fd = conn->fd;
	conn->connected = 0;
	conn->read_evt->data_size = sizeof(ain_conn_t*);
	*(ain_conn_t**)EVENT_DATA(conn->read_evt) = conn;
	if (AIN_OK != ain_socket_accept(conn->read_evt))
	{
		ain_conn_close(conn);
		return AIN_ERROR;
	}
	if (AIN_OK != ain_register_io_events(conn->ls->wdata->ioqe, &conn->read_evt, 1))
	{
		ain_conn_close(conn);
		return AIN_ERROR;
	}
	return AIN_OK;
}

void ain_conn_close(ain_conn_t* conn)
{
	ain_socket_close_grace(conn->fd);
	conn->fd = (ain_fd_t)-1;
	conn->time = conn->ls->wdata->time;
	if (conn->connected) {
		conn->proto.proto_handler(conn, NULL, 0);
		conn->connected = 0;
	}
}

ain_result_t ain_conn_onread(ain_event_t* evt)
{
	ain_conn_t* conn = *(ain_conn_t**)EVENT_DATA(evt);
	if (evt->buf_size == 0 || (evt->error && evt->_errno != AIN_IO_PENDING))
	{
		ain_conn_close(conn);
		return evt->error ? AIN_ERROR : AIN_OK;
	}

	if (!evt->error) {
		conn->time = conn->ls->wdata->time;
		ain_result_t res = conn->proto.proto_handler(conn, EVENT_BUFFER(evt), evt->buf_size);
		switch (res)
		{
		case AIN_OK:
			conn->write_evt->fd = conn->fd;
			return ain_socket_write(conn->write_evt);
		case AIN_ERROR:
			ain_conn_close(conn);
			return AIN_ERROR;
		}
	}

	return ain_socket_read(evt);
}

ain_result_t ain_conn_onwrite(ain_event_t* evt)
{
	ain_conn_t* conn = *(ain_conn_t**)EVENT_DATA(evt);
	size_t len = evt->buf_size;
	if (evt->error || len == 0)
	{
		ain_conn_close(conn);
		return evt->error ? AIN_ERROR : AIN_OK;
	}
	return ain_socket_read(conn->read_evt);
}
