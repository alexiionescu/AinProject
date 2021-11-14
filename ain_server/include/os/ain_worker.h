#pragma once

typedef struct _ain_worker_t ain_worker_t;

typedef ain_result_t (*ain_worker_proc_t)(void *);
ain_worker_t *ain_create_worker(ain_worker_proc_t proc, void *data,
                                uint32_t flags);
void ain_release_worker(ain_worker_t *worker);
