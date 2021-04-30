#include "ain_config.h"
#include "ain_globals.h"
#include "ain_ux.h"
#include "os/ain_memory.h"
#include "os/ain_sysinfo.h"
#include <sys/sysinfo.h>

static system_info_t* __sys_info = NULL;
system_info_t* get_sys_info()
{
	if (__sys_info) return __sys_info;
	static system_info_t sys_info;
	__sys_info = &sys_info;
	__sys_info->cpus = get_ncprocs();
	__sys_info->pageSize = getpagesize();
}
