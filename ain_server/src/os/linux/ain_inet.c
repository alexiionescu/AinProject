#include "ain_config.h"
#include "ain_globals.h"

const char *ain_inet_ntop(int af, const void *restrict src, char *restrict dst,
                          size_t dst_size) {
  const char *ret = inet_ntop(af, src, dst, dst_size);
  if (ret) {
    size_t addr_off = strlen(ret);
    if (dst_size - addr_off >= 6) {
      switch (af) {
      case AF_INET:
        sprintf(dst + addr_off, ":%hu",
                ntohs(((struct sockaddr_in *)src)->sin_port));
        break;
      case AF_INET6:
        sprintf(dst + addr_off, ":%hu",
                ntohs(((struct sockaddr_in6 *)src)->sin6_port));
        break;
      }
    }
  }
  return ret;
}