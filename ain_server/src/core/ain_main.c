#include "ain_config.h"
#include "ain_globals.h"
#include "os/ain_sysinfo.h"
#include "os/ain_process.h"
#include "core/ain_main_process.h"


uint32_t AIN_PAGE_SIZE;
int main(int argc, char** argv)
{
	ain_system_info_t* sysinfo = ain_get_sys_info();
	sysinfo->argc = argc;
	sysinfo->argv = argv;
	AIN_PAGE_SIZE = sysinfo->pageSize;

	if(ain_is_controller())
		return (int)ain_controller_process();

	if(ain_is_test_unit())
		return (int)ain_test_unit_process();

#ifdef _MSC_VER
	if (ain_is_debug_child())
		return (int)ain_child_process();

	ain_process_t* child = ain_process_fork();
	if (child)
		return (int)ain_parent_process(child);
	
#elif __GNUC__
	return (int)ain_parent_process_ux();
#endif	
}
