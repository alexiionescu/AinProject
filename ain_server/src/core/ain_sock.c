#include "ain_config.h"
#define _AIN_USE_SOCKETS_
#include "ain_globals.h"
#include "os/ain_socket.h"
#include "core/ain_protocol.h"
#include "core/ain_inet.h"
#include "core/ain_sock.h"
#include "core/ain_conf.h"

ain_listeners_t* ain_listeners_open(ain_conf_t* conf)
{
	AIN_ASSERT_NULL(conf->lls_size);
	ain_listeners_t* lsdata = (ain_listeners_t*)malloc(sizeof(ain_listeners_t));
	AIN_ASSERT_NULL(lsdata);
	lsdata->lls_size = conf->lls_size;
	lsdata->lls = (ain_sock_t*)calloc(lsdata->lls_size, sizeof(ain_sock_t));
	AIN_ASSERT_NULL(lsdata->lls);
	for (size_t l = 0; l < lsdata->lls_size; l++)
	{
		ain_sock_t* ls = lsdata->lls + l;
		uint8_t* buf = (uint8_t*)conf->lls[l].uri;
		size_t size = strlen(conf->lls[l].uri);
		if (AIN_OK != ain_proto_parse_uri(&ls->uri, &buf, &size))
			continue;
		if (conf->lls[l].conns_size)
			ls->conn_size = conf->lls[l].conns_size;
		else
			ls->conn_size = conf->w_conns_size;
		ls->wdata = NULL;
		ls->socktype = SOCK_STREAM; //TODO determine from uri
		ls->sockaddr.sa_family = AF_INET; //TODO determine from uri
		if(AIN_OK !=
			ain_inet_pton(ls->sockaddr.sa_family,
				ls->uri.scheme + ls->uri.host, ls->uri.host_size,
				&ls->sockaddr, &ls->sockaddrlen
			))
			continue;

		ain_inet_setport(ls->sockaddr.sa_family,
			ain_proto_get_default_port(ls->uri.scheme, ls->uri.scheme_size),
			ls->uri.scheme + ls->uri.port, ls->uri.port_size,
			&ls->sockaddr, ls->sockaddrlen);

		ain_listen(ls, conf->ls_backlog);
	}
	return lsdata;
}

ain_result_t ain_listen(ain_sock_t* ls, int backlog)
{
	ls->fd = (ain_fd_t)ain_socket_create(ls->sockaddr.sa_family, ls->socktype, 0);
	AIN_ASSERT_ERR(ls->fd != (ain_fd_t)-1);
	int reuseaddr = 1;
	AIN_ASSERT_ERR(-1 != setsockopt(ls->fd, SOL_SOCKET, SO_REUSEPORT,
		(const void*)&reuseaddr, sizeof(int)));
	AIN_ASSERT_ERR(-1 != bind(ls->fd, &ls->sockaddr, ls->sockaddrlen));
	AIN_ASSERT_ERR(-1 != listen(ls->fd, backlog));
	return AIN_OK;
}

void ain_listeners_close(ain_listeners_t* lsdata)
{
	for (size_t i = 0; i < lsdata->lls_size; i++)
	{
		ain_socket_close(lsdata->lls[i].fd);
	}
	free(lsdata->lls);
	free(lsdata);
}
