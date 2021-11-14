#pragma once

#include "core/ain_protocol.h"

struct _ain_sock_t {
	ain_fd_t			fd;
	struct sockaddr		sockaddr;
	socklen_t           sockaddrlen;    /* size of sockaddr */
	int                 socktype;
	worker_data_t		*wdata;
	uint16_t			conn_size;
	ain_uri_t			uri;
};

struct _ain_listeners_t {
	ain_sock_t		*lls;
	uint16_t		lls_size;
};

ain_listeners_t* ain_listeners_conf(ain_conf_t* conf);
ain_result_t ain_listen(ain_sock_t* ls, int back_log);
void ain_listeners_close(ain_listeners_t* lsdata);