#include "ain_config.h"
#include "ain_globals.h"

ain_result_t ain_inet_pton(int af, uint8_t* src, size_t src_size,
            struct sockaddr* dst, socklen_t* dstlen)
{
    ain_result_t ret = AIN_OK;
    uint8_t end = src[src_size];
    src[src_size] = 0;
    if (af == AF_INET)
    {
        *dstlen = sizeof(struct sockaddr_in);
        struct sockaddr_in* addr_in = (struct sockaddr_in*)dst;
        if (-1 == inet_pton(af, (const char*)src, &addr_in->sin_addr))
            ret = AIN_ERROR;
    }
    else if (af == AF_INET6)
    {
        *dstlen = sizeof(struct sockaddr_in6);
        struct sockaddr_in6* addr_in = (struct sockaddr_in6*)dst;
        if (-1 == inet_pton(af, (const char*)src, &addr_in->sin6_addr))
            ret = AIN_ERROR;
    }
    src[src_size] = end;
    return ret;
}

void ain_inet_setport(int af, uint16_t default_port, uint8_t* src, size_t src_size,
    struct sockaddr* dst, socklen_t dstlen)
{
    uint16_t port;
    AIN_TO_NUM(src, src_size, port);
    if (port == 0)
        port = default_port;
    if (af == AF_INET)
    {
        AIN_ASSERT(dstlen >= sizeof(struct sockaddr_in));
        struct sockaddr_in* addr_in = (struct sockaddr_in*)dst;
        addr_in->sin_port = htons(port);
    }
    else if (af == AF_INET6)
    {
        AIN_ASSERT(dstlen >= sizeof(struct sockaddr_in6));
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)dst;
        addr_in6->sin6_port = htons(port);
    }
}
