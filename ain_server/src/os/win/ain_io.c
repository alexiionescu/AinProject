#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_io.h"

struct _ain_io_t {
  HANDLE iocp;
};

ain_io_t *ain_create_io_queue() {
  ain_io_t *aio = (ain_io_t *)malloc(sizeof(ain_io_t));
  AIN_ASSERT_NULL(aio);
  aio->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
  AIN_ASSERT_NULL(aio->iocp);
  return aio;
}

void ain_destroy_io_queue(ain_io_t *aio) {
  CloseHandle(aio->iocp);
  free(aio);
}

void ain_register_io_events(ain_io_t *aio, ain_event_t **evtarray,
                            size_t buf_size) {
  // nothing todo on windows
}

ain_result_t ain_register_io_events_handle(ain_io_t *aio,
                                           ain_event_t **evtarray,
                                           size_t buf_size) {
  for (size_t i = 0; i < buf_size; i++) {
    AIN_ASSERT_ERR(CreateIoCompletionPort((HANDLE)evtarray[i]->fd, aio->iocp,
                                          (ULONG_PTR)0, 0));
  }
  return AIN_OK;
}

ain_result_t ain_wait_io_events(ain_io_t *aio, uint32_t timeout) {
  OVERLAPPED_ENTRY ov_array[IO_QUERY_CNT];
  ULONG count;
  if (!GetQueuedCompletionStatusEx(aio->iocp, ov_array, IO_QUERY_CNT, &count,
                                   timeout, FALSE)) {
    DWORD err = GetLastError();
    return (err == WAIT_TIMEOUT) ? AIN_TIMEOUT : AIN_ERROR;
  }

  for (ULONG i = 0; i < count; i++) {
    ain_event_t *evt = (ain_event_t *)ov_array[i].lpOverlapped;
    if (STATUS_WAIT_0 != ov_array[i].lpOverlapped->Internal) {
      evt->error = 1;
      evt->res = (ain_result_t)ov_array[i].lpOverlapped->Internal;
      if (evt->res == STATUS_PENDING) evt->res = AIN_IO_PENDING;
    } else {
      evt->error = 0;
      evt->res = 0;
    }
    evt->buf_size = ov_array[i].dwNumberOfBytesTransferred;
    evt->handler(evt);
  }
  return count;
}
