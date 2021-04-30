#pragma once

typedef struct _ain_process_t ain_process_t;

ain_process_t* ain_process_fork();
int ain_process_is_alive(ain_process_t* process);
void ain_process_kill(ain_process_t* child);
void ain_process_close(ain_process_t* child);
