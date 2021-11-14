#pragma once

ain_result_t ain_worker_proc(void *);
#define AIN_WORKER_RUN 0
#define AIN_WORKER_QUIT_REQ 1
#define AIN_WORKER_END 2
#define AIN_WORKER_QUIT_CMD 3
struct _worker_data_t {
  uint16_t id;
  ain_conf_t *conf;
  volatile uint8_t quit; // 0: running, 1: quiting,  2: quited
  ain_listeners_t *lsdata;
  uint16_t ls_assoc;
  ain_mem_pool_t *pool;
  ain_io_t *ioqe;
  time_t time;
};
