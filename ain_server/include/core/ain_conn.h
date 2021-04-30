#pragma once

#include "core/ain_protocol.h"

struct _ain_conn_t {
	ain_fd_t			fd;
	uint32_t			connected : 1;
	uint32_t			keep_alive : 1;
	ain_sock_t			*ls;

	ain_event_t			*read_evt;
	ain_event_t			*write_evt;
	time_t				time;
	ain_proto_data_t	proto;
};

ain_result_t ain_conn_init(ain_conn_t* conn, ain_sock_t* ls);
ain_result_t ain_conn_accept(ain_conn_t* conn);
ain_result_t ain_conn_onread(ain_event_t* evt);
ain_result_t ain_conn_onwrite(ain_event_t* evt);
void ain_conn_close(ain_conn_t* conn);