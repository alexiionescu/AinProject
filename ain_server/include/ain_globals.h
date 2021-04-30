#pragma once
//global C includes
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
//end global C includes

//ain global typedefs
typedef int ain_result_t;
extern uint32_t AIN_PAGE_SIZE;
//end ain global typedefs

//global objects typedefs
typedef struct _ain_mem_pool_t ain_mem_pool_t;
typedef struct _ain_io_t ain_io_t;
typedef struct _ain_event_t ain_event_t;
typedef struct _ain_sock_t ain_sock_t;
typedef struct _ain_uri_t ain_uri_t;
typedef struct _ain_conf_listener_t ain_conf_listener_t;
typedef struct _ain_conf_t ain_conf_t;
typedef struct _ain_listeners_t ain_listeners_t;
typedef struct _ain_conn_t ain_conn_t;
typedef struct _worker_data_t worker_data_t;
typedef struct _ain_proto_data_t ain_proto_data_t;
//end global objects typedefs

//OS Specific
#ifdef _MSC_VER
	#include "os/win/ain_windows.h"
#elif __GNUC__
	#include "os/linux/ain_ux.h"
#endif
//end OS Specific

//logging and asserting
#ifndef AIN_ASSERT_CONTINUE
#define AIN_ASSERT_CONTINUE	\
	PUSH_WARNINGS() \
	ain_result_t *__p = NULL; *__p = 0 \
	POP_WARNINGS()
#endif

#define AIN_ASSERT_START(cond) do { if(!(cond)) { \
					int err = (int)AIN_ERRNO; \
					fprintf(stderr,"%s:%d error %d\n",__FILE__, __LINE__, err); \
					AIN_ASSERT_CONTINUE;
#define AIN_ASSERT_END  } } while(0);

#define AIN_ASSERT(cond) AIN_ASSERT_START(cond) return; AIN_ASSERT_END
#define AIN_ASSERT_NULL(cond) AIN_ASSERT_START(cond) return NULL; AIN_ASSERT_END
#define AIN_ASSERT_ERR(cond) AIN_ASSERT_START(cond) return AIN_ERROR; AIN_ASSERT_END
//end logging and asserting

//global constants
#define AIN_OK				  0
#define AIN_ERROR			 -1
#define AIN_TIMEOUT			 -2
#define AIN_IO_PENDING		 -3

#define AIN_FOREVER 0xFFFFFFFF
//end global constants

//utils
#define AIN_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AIN_MAX(a,b) ((a) > (b) ? (a) : (b))

#define AIN_EQUALS(b,s,str) \
			(s == sizeof(str) - 1 && 0 == memcmp(b, str, s))
#define AIN_START_WITH(b,s,str) \
			(s >= sizeof(str) - 1 && 0 == memcmp(b, str, sizeof(str) - 1))
				
#define AIN_TO_NUM(b,s,n) \
				do { \
					n = 0; \
					while(s--) { \
						n *= 10; \
						n += *b++ - '0'; \
					} \
				} while (0)

#define AIN_BYTES_TO_PAGES(req) do { \
				if (req % AIN_PAGE_SIZE) \
					req = req / AIN_PAGE_SIZE + 1; \
				else \
					req = req / AIN_PAGE_SIZE; \
				} while(0)
//end utils
