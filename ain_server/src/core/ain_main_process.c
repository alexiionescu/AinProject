#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_io.h"
#include "os/ain_memory.h"
#include "os/ain_process.h"
#include "os/ain_socket.h"
#include "os/ain_sysinfo.h"
#include "os/ain_worker.h"

#include "core/ain_conf.h"
#include "core/ain_conn.h"
#include "core/ain_main_process.h"
#include "core/ain_sock.h"
#include "core/ain_worker_proc.h"

static volatile int _sig_stopped = 0;
int ain_sig_handler(int signal) {
  switch (signal) {
    case SIGINT:
      // printf("[Ctrl]+C\n");
      _sig_stopped = 1;
      return 1;
  };
  return 0;
}

ain_result_t ain_child_process() {
  ain_set_signal(ain_sig_handler);
  ain_system_info_t *sysinfo = ain_get_sys_info();
  ain_conf_t conf;
  AIN_ASSERT_ERR(AIN_OK == ain_config_load(&conf));
  if (conf.w_max == 0) conf.w_max = (uint8_t)sysinfo->cpus;
  ain_listeners_t *lsdata = ain_listeners_conf(&conf);
  uint16_t ls_size = lsdata->lls_size;
  for (size_t l = 0; l < ls_size; l++)
    ain_listen(lsdata->lls + l, conf.ls_backlog);
  AIN_ASSERT_ERR(lsdata);
  ain_worker_t **workers =
      (ain_worker_t **)malloc(conf.w_max * sizeof(ain_worker_t *));
  AIN_ASSERT_ERR(workers);
  int shm_id;
  worker_data_t *workers_data = (worker_data_t *)ain_shared_data_alloc(
      conf.w_max, sizeof(worker_data_t), &shm_id);
  AIN_ASSERT_ERR(workers_data);

  for (uint8_t i = 0; i < conf.w_max; i++) {
    worker_data_t *wdata = workers_data + i;
    wdata->conf = &conf;
    wdata->id = i;
    wdata->lsdata = lsdata;
    for (size_t l = i; l < ls_size; l += conf.w_max) {
      ain_sock_t *ls = lsdata->lls + l;
      ls->wdata = wdata;
      wdata->ls_assoc++;
    }
    workers[i] = ain_create_worker(ain_worker_proc, wdata, 0);
  }

  int join;
  do {
    ain_sleep(50);
    join = conf.w_max;
    for (uint32_t i = 0; i < conf.w_max; i++) {
      switch (workers_data[i].quit) {
        case AIN_WORKER_END:
          join--;
          break;
        case AIN_WORKER_QUIT_CMD:
          for (uint32_t j = 0; j < conf.w_max; j++) {
            workers_data[j].quit = AIN_WORKER_QUIT_REQ;
          }
          break;
        case AIN_WORKER_QUIT_REQ:
          break;
        default:
          if (_sig_stopped) workers_data[i].quit = AIN_WORKER_QUIT_REQ;
      }
    }
  } while (join);

  ain_listeners_close(lsdata);
  for (uint8_t i = 0; i < conf.w_max; i++) ain_release_worker(workers[i]);
  free(workers);
  ain_shared_data_free(workers_data, &shm_id);
  ain_config_release(&conf);
  return AIN_OK;
}

ain_result_t ain_parent_process_ux() {
  ain_set_signal(ain_sig_handler);
  ain_system_info_t *sysinfo = ain_get_sys_info();
  ain_conf_t conf;
  AIN_ASSERT_ERR(AIN_OK == ain_config_load(&conf));
  if (conf.w_max == 0) conf.w_max = (uint8_t)sysinfo->cpus;
  ain_listeners_t *lsdata = ain_listeners_conf(&conf);
  AIN_ASSERT_ERR(lsdata);
  int shm_id;
  worker_data_t *workers_data = (worker_data_t *)ain_shared_data_alloc(
      conf.w_max, sizeof(worker_data_t), &shm_id);
  AIN_ASSERT_ERR(workers_data);
  ain_process_t **workers =
      (ain_process_t **)malloc(conf.w_max * sizeof(ain_process_t *));
  for (uint8_t i = 0; i < conf.w_max; i++) {
    if (NULL == (workers[i] = ain_process_fork())) {
      printf("\n%02hu child started\n", i);
      worker_data_t *wdata = workers_data + i;
      wdata->conf = &conf;
      wdata->id = i;

      wdata->lsdata = lsdata;
      uint16_t ls_size = lsdata->lls_size;
      for (size_t l = i; l < ls_size; l++) {
        ain_sock_t *ls = lsdata->lls + l;
        ain_listen(ls, conf.ls_backlog);
        ls->wdata = wdata;
        wdata->ls_assoc++;
      }
      ain_result_t ret = ain_worker_proc(wdata);
      ain_listeners_close(lsdata);
      printf("\n%02hu child stopped\n", i);
      return ret;
    }
  }

  int join;
  do {
    ain_sleep(50);
    join = conf.w_max;
    for (uint32_t i = 0; i < conf.w_max; i++) {
      switch (workers_data[i].quit) {
        case AIN_WORKER_END:
          join--;
          break;
        case AIN_WORKER_QUIT_CMD:
          for (uint32_t j = 0; j < conf.w_max; j++) {
            workers_data[j].quit = AIN_WORKER_QUIT_REQ;
          }
          break;
        case AIN_WORKER_QUIT_REQ:
          break;
        default:
          if (_sig_stopped) workers_data[i].quit = AIN_WORKER_QUIT_REQ;
      }
    }
  } while (join);

  for (uint8_t i = 0; i < conf.w_max; i++) ain_process_close(workers[i]);
  free(workers);
  ain_shared_data_free(workers_data, &shm_id);
  ain_config_release(&conf);
  return AIN_OK;
}

ain_result_t ain_parent_process(ain_process_t *child) {
  // ain_io_t* ioqe = ain_create_io_queue();
  // ain_mem_pool_t* pool = ain_pool_create(0,0);

  ain_set_signal(ain_sig_handler);
  // TODO recv command from controller using parent mgmt server
  // and send them to child process mgmt server
  do {
    // ain_wait_io_events(ioqe, 50);
    ain_sleep(50);
    if (!ain_process_is_alive(child)) {
      ain_process_close(child);
      child = ain_process_fork();
    }
  } while (!_sig_stopped);

  ain_process_close(child);
  // ain_destroy_io_queue(ioqe);
  // ain_pool_destroy(pool);
  return AIN_OK;
}

ain_result_t ain_controller_process() {
  ain_io_t *ioqe = ain_create_io_queue();
  ain_mem_pool_t *pool = ain_pool_create(0, 0);

  // TODO send command to parent process

  ain_destroy_io_queue(ioqe);
  ain_pool_destroy(pool);
  return AIN_OK;
}

int ain_is_debug_child() {
  ain_system_info_t *sysinfo = ain_get_sys_info();
  if (sysinfo->argc > 1 && !memcmp(sysinfo->argv[1], "debug_child_process",
                                   sizeof("debug_child_process")))
    return 1;
  return 0;
}

int ain_is_controller() {
  ain_system_info_t *sysinfo = ain_get_sys_info();
  if (sysinfo->argc > 1 && !memcmp(sysinfo->argv[1], "controller_process",
                                   sizeof("controller_process")))
    return 1;
  return 0;
}

int ain_is_test_unit() {
  ain_system_info_t *sysinfo = ain_get_sys_info();
  if (sysinfo->argc > 1 && !memcmp(sysinfo->argv[1], "test_unit_process",
                                   sizeof("test_unit_process")))
    return 1;
  return 0;
}

extern void testsUnits();
ain_result_t ain_test_unit_process() {
  testsUnits();
  return 0;
}
