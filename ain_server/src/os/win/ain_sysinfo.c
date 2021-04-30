#include "ain_config.h"
#include "ain_globals.h"
#include "os/ain_memory.h"
#include "os/ain_sysinfo.h"
#include "os/ain_socket.h"


static ain_system_info_t* __sys_info = NULL;
ain_system_info_t* ain_get_sys_info()
{
	if (__sys_info) return __sys_info;
	static ain_system_info_t sys_info;
	__sys_info = &sys_info;

	SYSTEM_INFO os_info;
	GetSystemInfo(&os_info);
	__sys_info->cpus = os_info.dwNumberOfProcessors;
	__sys_info->pageSize = os_info.dwPageSize;
	ain_sockets_init();
	return __sys_info;
}
