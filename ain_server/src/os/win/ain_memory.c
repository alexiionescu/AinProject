#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_memory.h"

void *ain_shared_data_alloc(size_t num, size_t size, int *shm_id) {
  (shm_id);
  return calloc(num, size);
}

void ain_shared_data_free(void *wdata, int *shm_id) {
  (shm_id);
  free(wdata);
}