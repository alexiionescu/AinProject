#pragma once

#define AIN_ERRNO  errno
typedef int ain_fd_t;

#ifdef _AIN_SOCKET_FILE_
    #include <sys/socket.h>
    #define ain_socket(af, type, proto)  socket(af, type, proto)
    #define ain_close_socket(fd) close(fd)
#endif    

#define PUSH_WARNINGS() \
        _Pragma ("GCC diagnostic push") \
        _Pragma ("GCC diagnostic ignored \"-Wall\"")
#define POP_WARNINGS() \
		_Pragma ("GCC diagnostic pop")