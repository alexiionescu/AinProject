#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_file.h"

#include "core/ain_event.h"

ain_result_t ain_file_write(ain_event_t *evt) {
  memset(evt, 0, sizeof(OVERLAPPED));
  BOOL bRet = WriteFile((HANDLE)evt->fd, EVENT_BUFFER(evt), evt->buf_size, NULL,
                        (LPOVERLAPPED)evt);
  if (bRet) {
    evt->buf_size = (uint32_t)evt->iocp_id.InternalHigh;
    evt->handler(evt);
  }
  return (bRet || GetLastError() == ERROR_IO_PENDING) ? AIN_OK : AIN_ERROR;
}

ain_result_t ain_file_read(ain_event_t *evt) {
  memset(evt, 0, sizeof(OVERLAPPED));
  BOOL bRet = ReadFile((HANDLE)evt->fd, EVENT_BUFFER(evt),
                       EVENT_BUFFER_SIZE(evt), NULL, (LPOVERLAPPED)evt);
  if (bRet) {
    evt->buf_size = (uint32_t)evt->iocp_id.InternalHigh;
    evt->handler(evt);
  }
  return (bRet || GetLastError() == ERROR_IO_PENDING) ? AIN_OK : AIN_ERROR;
}

void ain_file_close(ain_fd_t fd) { CloseHandle((HANDLE)fd); }
