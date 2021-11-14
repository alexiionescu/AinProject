#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_memory.h"

#include "core/ain_event.h"

ain_event_t *ain_event_alloc(ain_mem_pool_t *pool, size_t data_size,
                             size_t pages, uint32_t blocks) {
  ain_event_t *evt = (ain_event_t *)malloc(sizeof(ain_event_t) + data_size +
                                           sizeof(struct iovec) * blocks);
  AIN_ASSERT_NULL(evt);
  evt->iovcnt = blocks;
  evt->extrasize = pages - 1;
  evt->data_size = data_size;
  for (int i = 0; i < blocks; i++) {
    evt->iov[i].iov_base = ain_pool_alloc(pool, pages);
    AIN_ASSERT_NULL(evt->iov[i].iov_base);
    evt->iov[i].iov_len = AIN_PAGE_SIZE * pages;
  }
  return evt;
}

void ain_event_free(ain_mem_pool_t *pool, ain_event_t *evt) {
  for (int i = 0; i < evt->iovcnt; i++)
    ain_pool_free(pool, evt->iov[i].iov_base);
  free(evt);
}