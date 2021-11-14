#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_worker.h"

struct _ain_worker_t {
  HANDLE thread;
  DWORD threadId;
  uint32_t flags;
  ain_worker_proc_t proc;
  void *data;
};

static DWORD WINAPI MyThreadHandler(LPVOID lpParam) {
  ain_worker_t *_w = (ain_worker_t *)lpParam;
  return (DWORD)_w->proc(_w->data);
}

ain_worker_t *ain_create_worker(ain_worker_proc_t proc, void *data,
                                uint32_t flags) {
  ain_worker_t *_w = (ain_worker_t *)malloc(sizeof(ain_worker_t));
  AIN_ASSERT_NULL(_w);
  _w->data = data;
  _w->flags = flags;
  _w->proc = proc;
  _w->thread = CreateThread(NULL, 0, MyThreadHandler, _w, 0, &_w->threadId);
  AIN_ASSERT_NULL(_w->thread);
  return _w;
}

void ain_release_worker(ain_worker_t *worker) {}
