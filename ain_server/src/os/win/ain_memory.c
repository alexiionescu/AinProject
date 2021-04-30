#include "ain_config.h"
#include "ain_globals.h"
#include "os/ain_memory.h"

PUSH_WARNINGS()
#pragma pack(1)
struct _ain_mem_pool_t {
	uint16_t pool_size;
	uint32_t max_pools;
	uint32_t pool_cnt;
};
typedef struct _ain_memory_pool_t {
	void* base;
	uint16_t pages[]; /*pages that are free have 0, 
									occupied pages have 1 or consecutive page no*/
}ain_memory_pool_t;
POP_WARNINGS()
#pragma pack()
#define SIZEOF_MEMORY_POOL(pool_size) \
			(sizeof(ain_memory_pool_t) + pool_size * sizeof(uint16_t))
#define GET_FIRST_POOL(pool) \
			(ain_memory_pool_t*)((uint8_t*)pool+sizeof(ain_mem_pool_t))
#define GET_NEXT_POOL(pool,_pool) \
			(ain_memory_pool_t*)((uint8_t*)pool+SIZEOF_MEMORY_POOL(_pool->pool_size))

ain_mem_pool_t* ain_pool_create(uint16_t pool_size, uint32_t max_pools)
{
	if (pool_size < AIN_MIN_POOL_SIZE)
		pool_size = AIN_MIN_POOL_SIZE;
	if (max_pools == 0)
		max_pools = 1; //at least 1 pool
	size_t req = (sizeof(ain_mem_pool_t) + max_pools * SIZEOF_MEMORY_POOL(pool_size));
	AIN_BYTES_TO_PAGES(req);

	ain_mem_pool_t* _pool = VirtualAlloc(NULL, (pool_size) * AIN_PAGE_SIZE,
								MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	AIN_ASSERT_NULL(_pool);
	_pool->pool_size = pool_size;
	_pool->max_pools = max_pools;
	_pool->pool_cnt = 1;
	ain_memory_pool_t* pool = GET_FIRST_POOL(_pool);
	pool->base = _pool;
	// alloc space for config data
	for (size_t k = 0; k < req; k++)
		pool->pages[k] = (uint8_t)(1 + k);
	return _pool;
}

void* ain_pool_alloc(ain_mem_pool_t* _pool, size_t req)
{
	size_t i = 0, j = 0;
	ain_memory_pool_t* pool = GET_FIRST_POOL(_pool);
	while (j - i < req) {
		while (i < _pool->pool_size && pool->pages[i])
			i++;
		if (i < _pool->pool_size)
		{
			j = i + 1;
			while (j < _pool->pool_size && !pool->pages[j] && j - i < req)
				j++;
		}
		if (i == _pool->pool_size || (j == _pool->pool_size && j - i < req))
		{
			ain_memory_pool_t* npool = GET_NEXT_POOL(pool,_pool);
			if (!npool->base) {
				AIN_ASSERT_NULL(_pool->pool_cnt < _pool->max_pools);
				npool->base = VirtualAlloc(NULL, _pool->pool_size * AIN_PAGE_SIZE,
									MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				AIN_ASSERT_NULL(npool->base);
				_pool->pool_cnt++;
			}
			pool = npool;
			
			i = 0; 
			j = 0;
		}
	}
	for (size_t k=i; k < j; k++)
		pool->pages[k] = (uint8_t)(1 + k - i);
	return (uint8_t*)pool->base + i * AIN_PAGE_SIZE;
}

void ain_pool_free(ain_mem_pool_t* _pool, void* p)
{
	ain_memory_pool_t* pool = GET_FIRST_POOL(_pool);
	int32_t k = (int32_t)((uint8_t*)p - (uint8_t*)pool->base) / AIN_PAGE_SIZE;
	while (k < 0 || k >= _pool->pool_size) {
		pool = GET_NEXT_POOL(pool, _pool);
		AIN_ASSERT(pool->base);
		k = (int32_t)((uint8_t*)p - (uint8_t*)pool->base) / AIN_PAGE_SIZE;
	}
	AIN_ASSERT(pool->pages[k]==1)
	while (k < (int32_t)_pool->pool_size - 1 && pool->pages[k] < pool->pages[k + 1])
		pool->pages[k++] = 0;
	if(k < (int32_t)_pool->pool_size)
		pool->pages[k] = 0;
}

void ain_pool_destroy(ain_mem_pool_t* _pool)
{
	ain_memory_pool_t* pool = GET_FIRST_POOL(_pool);
	for (uint32_t i = 1; i < _pool->pool_cnt; i++) 
	{
		pool = GET_NEXT_POOL(pool, _pool);
		VirtualFree(pool->base, 0, MEM_RELEASE);
	}
	VirtualFree(_pool,0,MEM_RELEASE);
}
