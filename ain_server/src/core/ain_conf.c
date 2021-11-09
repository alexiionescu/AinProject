#include "ain_config.h"
#include "ain_globals.h"
#include "core/ain_conf.h"

ain_result_t ain_config_load(ain_conf_t* conf)
{
	conf->lls_size = 2;
	conf->lls = (ain_conf_listener_t*)malloc(conf->lls_size*sizeof(ain_conf_listener_t));
	AIN_ASSERT_ERR(conf->lls);
	conf->lls[0].uri = strdup("http://0.0.0.0:9081");
	conf->lls[0].conns_size = 0;
	conf->lls[1].uri = strdup(AIN_MGMT_CHILD_ADDRESS);
	conf->lls[1].conns_size = 4;

	conf->ls_backlog = AIN_LISTEN_BACKLOG;
	conf->w_max = 	AIN_DEFAULT_WORKERS;
	uint16_t w_csize = 0;
	uint16_t total_conn_size = AIN_WORKER_CONNECTIONS;
	for (size_t i = 0; i < conf->lls_size; i++) {
		if (conf->lls[i].conns_size)
			total_conn_size -= conf->lls[i].conns_size;
		else
			w_csize++;
	}
	conf->w_conns_size = total_conn_size / w_csize;
	conf->w_mem_pool_size = AIN_POOL_SIZE;
	conf->w_mem_max_pools = AIN_MAX_POOLS;
	return AIN_OK;
}

void ain_config_release(ain_conf_t* conf)
{
	for (size_t i = 0; i < conf->lls_size; i++) {
		free(conf->lls[i].uri);
	}
	free(conf->lls);
}

