#include "ain_config.h"
#include "ain_globals.h"
#include "os/ain_memory.h"
#include "os/ain_sysinfo.h"
#include "os/ain_worker.h"
#include "os/ain_process.h"
#include "os/ain_socket.h"
#include "os/ain_io.h"
#include "core/ain_conf.h"
#include "core/ain_conn.h"
#include "core/ain_sock.h"
#include "core/ain_worker_proc.h"

ain_result_t ain_listen_event(ain_event_t* evt)
{
	(evt);
	return AIN_OK;
}

ain_result_t ain_worker_proc(void* data) {
	worker_data_t* wdata = (worker_data_t*)data;

	uint32_t req = 0;
	uint16_t pool_size = 16;
	ain_conn_t* w_conns = NULL;
	uint16_t ls_size = wdata->lsdata->lls_size;

	for (size_t l = 0; l < ls_size; l++)
	{
		ain_sock_t* ls = wdata->lsdata->lls + l;
		if (ls->wdata != wdata)
			continue;
		req += ls->conn_size * sizeof(ain_conn_t);
		pool_size += 2 * ls->conn_size; //min 2 pages per connection
	}
	wdata->ioqe = ain_create_io_queue();
	wdata->pool = ain_pool_create(
					AIN_MAX(pool_size, wdata->conf->w_mem_pool_size),
					wdata->conf->w_mem_max_pools);
	if (req) {
		AIN_BYTES_TO_PAGES(req);
		w_conns = ain_pool_alloc(wdata->pool, req);
		ain_conn_t* conn = w_conns;

		for (size_t l = 0; l < ls_size; l++)
		{
			ain_sock_t* ls = wdata->lsdata->lls + l;
			if (ls->wdata != wdata)
				continue;

			ain_event_t* lsevt = ain_pool_alloc(wdata->pool, 1);
			lsevt->fd = ls->fd;
			lsevt->handler = ain_listen_event;
			ain_register_io_events(wdata->ioqe, &lsevt, 1);

			for (size_t c = 0; c < ls->conn_size; c++)
			{
				ain_conn_init(conn, ls);
				ain_conn_accept(conn);
				conn++;
			}
		}
	}
	do {
		ain_result_t ior = ain_wait_io_events(wdata->ioqe, 50);
		if (IO_QUERY_CNT != ior) {
			if (ior == AIN_TIMEOUT) { //nothing on io
				time(&wdata->time);  //update worker time
			}
			if (w_conns)
			{
				ain_conn_t* conn = w_conns;
				for (size_t l = 0; l < ls_size; l++)
				{
					ain_sock_t* ls = wdata->lsdata->lls + l;
					if (ls->wdata != wdata)
						continue;

					for (size_t c = 0; c < ls->conn_size; c++)
					{
						if (conn->fd == (ain_fd_t)-1 
								&& wdata->time - conn->time > 0)
							ain_conn_accept(conn); //re-use closed socket
						else if (conn->keep_alive && 
									conn->connected && wdata->time - conn->time > 5)
							ain_conn_close(conn); //keep-alive close
						conn++;
					}
				}
			}
		}
		if (wdata->quit == AIN_WORKER_QUIT_REQ)
			break;
	} while (1);

	if (w_conns)
	{
		ain_conn_t* conn = w_conns;
		for (size_t l = 0; l < ls_size; l++)
		{
			ain_sock_t* ls = wdata->lsdata->lls + l;
			if (ls->wdata != wdata)
				continue;

			for (size_t c = 0; c < ls->conn_size; c++)
			{
				ain_conn_close(conn);
				conn++;
			}
		}
	}

	ain_destroy_io_queue(wdata->ioqe);
	ain_pool_destroy(wdata->pool);
	wdata->quit = AIN_WORKER_END;
	return AIN_OK;
}