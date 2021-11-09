#pragma once

typedef struct _ain_process_t ain_process_t;

ain_process_t* ain_process_fork();
int ain_process_is_alive(ain_process_t* process);
void ain_process_stop(ain_process_t* child);
void ain_process_close(ain_process_t* child);

typedef int (*ain_sighandler_t)(int);
void ain_set_signal(ain_sighandler_t handler);