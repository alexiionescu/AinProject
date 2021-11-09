#pragma once
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>

#define AIN_ERRNO  errno
#define INVALID_SOCKET (-1)
typedef int ain_fd_t;

#define DO_PRAGMA(x) _Pragma (#x)

#ifdef _AIN_USE_SOCKETS_
    #define ain_socket_create(af, type, proto)  socket(af, type, proto)
    #define ain_socket_close(fd) close(fd)
    #define ain_socket_close_grace(fd)	do { shutdown(fd,SHUT_RDWR);close(fd); }while(0)

#endif    

#define ain_sleep(ms)   usleep(ms*1000)

#define PUSH_WARNINGS()
#define POP_WARNINGS()

#include <liburing.h>
struct _ain_io_t
{
    struct io_uring ring;
};