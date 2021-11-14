#pragma once

#include "config.h"

#ifndef AIN_DEFAULT_EVT_SIZE
#define AIN_DEFAULT_EVT_SIZE 1 // default buffer pages for an event
#endif

#ifndef AIN_MIN_POOL_SIZE
#define AIN_MIN_POOL_SIZE 16 // pages
#endif

#ifndef AIN_POOL_SIZE
#define AIN_POOL_SIZE 256 // pages, 256 = 1MB
#endif

#ifndef AIN_MAX_POOLS
#define AIN_MAX_POOLS 32 // max no of pools
#endif

#ifndef AIN_LISTEN_BACKLOG
#define AIN_LISTEN_BACKLOG 511 // max no of pools
#endif

#ifndef AIN_WORKER_CONNECTIONS
#define AIN_WORKER_CONNECTIONS 512
// total no of connections (sockets) for all listeners
#endif

#ifndef AIN_DEFAULT_WORKERS
#define AIN_DEFAULT_WORKERS 0 // 0 for core count
#endif

#ifndef AIN_MGMT_CHILD_ADDRESS
#define AIN_MGMT_CHILD_ADDRESS "ainc://0.0.0.0:39480;transport=tcp"
// default mgmt adress of child process
#endif

#ifndef AIN_MGMT_PARENT_ADDRESS
#define AIN_MGMT_PARENT_ADDRESS "ainp://0.0.0.0:39481;transport=tcp"
// default mgmt adress of parent process
#endif

#ifndef IO_QUERY_CNT
#define IO_QUERY_CNT 20
#endif

// OS Specific part
#ifdef _MSC_VER
#elif __GNUC__
#endif
