#include "ain_config.h"
#include "ain_globals.h"
#include "os/ain_process.h"
#include "os/ain_sysinfo.h"

struct _ain_process_t {
	HANDLE process;
	DWORD processId;
	HANDLE thread;
	DWORD threadId;
};

ain_process_t* ain_process_fork()
{
	ain_system_info_t* sysinfo = ain_get_sys_info();
	if (!memcmp(sysinfo->argv[0], "ain_child_process", sizeof("ain_child_process")))
		return NULL;

	ain_process_t* ps = (ain_process_t*)malloc(sizeof(ain_process_t));
	AIN_ASSERT_NULL(ps);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	
	AIN_ASSERT_NULL(CreateProcess(sysinfo->argv[0], "ain_child_process", 
						NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi));
	ps->process = pi.hProcess;
	ps->processId = pi.dwProcessId;
	ps->thread = pi.hThread;
	ps->threadId = pi.dwThreadId;
	return ps;
}

int ain_process_is_alive(ain_process_t* p)
{
	if (WaitForSingleObject(p->process, 0) == WAIT_TIMEOUT)
		return 1;

	return 0;
}

void ain_process_kill(ain_process_t* child)
{
	TerminateProcess(child->process,1);
}

void ain_process_close(ain_process_t* child)
{
	CloseHandle(child->process);
	CloseHandle(child->thread);
	free(child);
}


