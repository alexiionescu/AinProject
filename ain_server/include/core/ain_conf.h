#pragma once

struct _ain_conf_listener_t {
	char		*uri;
	uint16_t	conns_size;
};
struct _ain_conf_t {
	ain_conf_listener_t *lls;
	uint16_t			lls_size;
	int					ls_backlog;
	uint16_t			w_conns_size;
	uint16_t			w_mem_pool_size;
	uint16_t			w_mem_max_pools;
	uint8_t				w_max;
};
ain_result_t ain_config_load(ain_conf_t* conf);
void ain_config_release(ain_conf_t* conf);
