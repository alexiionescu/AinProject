#include "ain_config.h"
#include "ain_globals.h"

#include <sys/utsname.h>

#include "os/ain_io.h"

#define MIN_KERNEL_VERSION 5
#define MIN_MAJOR_VERSION 5

static int check_kernel_version() {
  static long ver[16] = {0};
  if (ver[0] >= MIN_KERNEL_VERSION && ver[1] >= MIN_MAJOR_VERSION) {
    return 0;  // already checked
  }
  struct utsname buffer;
  char *p;
  int i = 0;

  if (uname(&buffer) != 0) {
    perror("uname");
    exit(EXIT_FAILURE);
  }

  p = buffer.release;

  while (*p) {
    if (isdigit(*p)) {
      ver[i] = strtol(p, &p, 10);
      i++;
    } else {
      p++;
    }
  }

  if (ver[0] >= MIN_KERNEL_VERSION && ver[1] >= MIN_MAJOR_VERSION) {
    printf("Your kernel version is: %ld.%ld\n", ver[0], ver[1]);
    return 0;
  }
  fprintf(stderr, "Error: your kernel version is: %ld.%ld\n", ver[0], ver[1]);
  exit(EXIT_FAILURE);
  return -1;
}

#define QUEUE_DEPTH 512

ain_io_t *ain_create_io_queue() {
  AIN_ASSERT_NULL(0 == check_kernel_version());
  ain_io_t *aio = (ain_io_t *)malloc(sizeof(ain_io_t));
  memset(aio, 0, sizeof(ain_io_t));
  AIN_ASSERT_NULL(aio);
  struct io_uring_params params;
  memset(&params, 0, sizeof(params));
  AIN_ASSERT_NULL(-1 !=
                  io_uring_queue_init_params(QUEUE_DEPTH, &aio->ring, &params));

  if (!(params.features & IORING_FEAT_FAST_POLL)) {
    printf("[%d] Warning: IORING_FEAT_FAST_POLL not available in the kernel\n",
           getpid());
  }

  return aio;
}

void ain_destroy_io_queue(ain_io_t *aio) {
  io_uring_queue_exit(&aio->ring);
  free(aio);
}

void ain_register_io_events(ain_io_t *aio, ain_event_t **evtarray,
                            size_t buf_size) {
  for (size_t i = 0; i < buf_size; i++) {
    evtarray[i]->aio = aio;
  }
}

ain_result_t ain_register_io_events_handle(ain_io_t *aio,
                                           ain_event_t **evtarray,
                                           size_t buf_size) {
  return AIN_OK;  // nothing to do on linux
}

ain_result_t ain_wait_io_events(ain_io_t *aio, uint32_t timeout) {
  struct io_uring_cqe *cqe;
  struct __kernel_timespec ts;
  ts.tv_sec = timeout / 1000;
  ts.tv_nsec = (timeout % 1000) * 1000000;

  int i = 0;
  do {
    int ret = io_uring_wait_cqes(&aio->ring, &cqe, IO_QUERY_CNT, &ts, NULL);
    if (ret == -ETIME) break;
    if (ret < 0) {
      fprintf(stderr, "%s: wait timeout failed: %d\n", __FUNCTION__, ret);
      goto err;
    }

    ain_event_t *evt = (ain_event_t *)cqe->user_data;
    evt->res = cqe->res;
    evt->cqe_flags = cqe->flags;
    if (evt->res < 0) {
      evt->error = 1;
      fprintf(stderr, "ain_wait_io_events res: %d '%s'\n", -evt->res,
              strerror(-evt->res));
      // if (evt->res == STATUS_PENDING)
      //     evt->res = AIN_IO_PENDING;
    } else {
      evt->error = 0;
    }
    evt->handler(evt);
    io_uring_cqe_seen(&aio->ring, cqe);
    i++;
  } while (1);
  return i;
err:
  return AIN_ERROR;
}