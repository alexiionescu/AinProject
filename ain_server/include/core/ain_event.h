#pragma once


typedef ain_result_t (*event_handler_t)(ain_event_t*);

struct _ain_event_t {
#ifdef _MSC_VER
	OVERLAPPED		iocp_id;
#endif
	event_handler_t handler;
	ain_result_t	_errno; //last IO operation error if error is 1
	ain_fd_t		fd;
	uint32_t		error	  : 1;
	uint32_t		extrasize : 12;  //extra allocated pages in pool
	uint32_t		data_size : 12;	//data size
	uint32_t		buf_size;		//io buffer size
#define EVENT_DATA(evt)			((uint8_t*)evt + sizeof(ain_event_t))  //event data structure
#define EVENT_DATA_SIZE(evt)	(evt->data_size + sizeof(ain_event_t))  //event data size
#define EVENT_BUFFER(evt)		((uint8_t*)evt + EVENT_DATA_SIZE(evt))
#define EVENT_BUFFER_SIZE(evt)	(AIN_PAGE_SIZE*(1+evt->extrasize) - EVENT_DATA_SIZE(evt))
};

			
			
