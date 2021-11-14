#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_memory.h"
#include "os/ain_sysinfo.h"

static ain_system_info_t *__sys_info = NULL;
ain_system_info_t *ain_get_sys_info() {
  if (__sys_info) return __sys_info;
  static ain_system_info_t sys_info;
  __sys_info = &sys_info;

  __sys_info->cpus = sysconf(_SC_NPROCESSORS_CONF);
  __sys_info->pageSize = (uint32_t)getpagesize();
  return __sys_info;
}
