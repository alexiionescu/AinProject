#pragma once

typedef struct _ain_system_info_t {
	uint32_t cpus;
	uint32_t pageSize;
	int argc;
	char** argv;
}ain_system_info_t;

ain_system_info_t* ain_get_sys_info();
