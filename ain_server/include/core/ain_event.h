#pragma once


typedef ain_result_t (*event_handler_t)(ain_event_t*);

struct _ain_event_t {
#ifdef _MSC_VER
	OVERLAPPED		iocp_id;
#elif __GNUC__
	ain_io_t* 		aio;
	uint32_t		cqe_flags;
#endif
	event_handler_t handler; //this handler processes the io response
	ain_result_t	res;     //last IO operation result or error (if error is 1)
	ain_fd_t		fd;
	uint32_t		error	  : 1;
	uint32_t		extrasize : 12;  //extra allocated pages in pool
	uint32_t		data_size : 12;	 //data size
	size_t			buf_size;		 //io buffer size
	uint32_t		iovcnt;
	struct iovec	iov[];
#define EVENT_DATA(evt)			((uint8_t*)evt  + sizeof(ain_event_t) + evt->iovcnt * sizeof(struct iovec))  //event data structure offset
#define EVENT_DATA_SIZE(evt)	(evt->data_size + sizeof(ain_event_t) + evt->iovcnt * sizeof(struct iovec))  //event data size
#define EVENT_BUFFER(evt)  		(evt->iov[0].iov_base)	
#define EVENT_BUFFER_SIZE(evt)	(evt->iov[0].iov_len)	
};

ain_event_t* ain_event_alloc(ain_mem_pool_t*pool, size_t data_size, size_t pages_per_block, uint32_t blocks);
void ain_event_free(ain_mem_pool_t*pool, ain_event_t* evt);

