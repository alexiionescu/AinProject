#pragma once

#include "core/ain_event.h"
ain_io_t* ain_create_io_queue();
void ain_destroy_io_queue(ain_io_t* aio);	
ain_result_t ain_register_io_events(ain_io_t* aio, ain_event_t** evtarray, size_t buf_size);
ain_result_t ain_wait_io_events(ain_io_t* aio, uint32_t timeout);
