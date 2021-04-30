#pragma once
ain_mem_pool_t* ain_pool_create(uint16_t pool_size, uint32_t max_pools);
void* ain_pool_alloc(ain_mem_pool_t* pool, size_t req);
void ain_pool_free(ain_mem_pool_t* pool, void* p);
void ain_pool_destroy(ain_mem_pool_t* pool);
