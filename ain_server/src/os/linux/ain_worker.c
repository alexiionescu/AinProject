#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_worker.h"

// TODO -- implement with pthead
#include <sys/shm.h>

struct _ain_worker_t {};

ain_worker_t *ain_create_worker(ain_worker_proc_t proc, void *data,
                                uint32_t flags) {
  AIN_ASSERT_NULL(0);
}

void ain_release_worker(ain_worker_t *worker) {}