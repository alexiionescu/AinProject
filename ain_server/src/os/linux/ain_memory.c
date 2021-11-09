#include "ain_config.h"
#include "ain_globals.h"
#include "os/ain_worker.h"

#include <sys/shm.h>

void* ain_shared_data_alloc(size_t num, size_t size, int* shm_id)
{
    *shm_id = shmget(IPC_PRIVATE, num * size, IPC_CREAT | 0666);
    AIN_ASSERT_NULL(*shm_id >= 0);
    void* data = shmat(*shm_id, NULL, 0);
    AIN_ASSERT_NULL(data != (void *)-1);
    memset(data, 0 , num * size);
	return data;
}

void ain_shared_data_free(void* wdata, int* shm_id)
{
    shmdt(wdata);
	shmctl(*shm_id, IPC_RMID, NULL);
    *shm_id = 0;
}