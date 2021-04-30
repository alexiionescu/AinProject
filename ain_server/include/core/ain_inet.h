#pragma once

ain_result_t ain_inet_pton(int af, uint8_t* src, size_t src_size,
	struct sockaddr* dst, socklen_t* dstlen);
void ain_inet_setport(int af, uint16_t default_port, uint8_t* src, size_t src_size,
	struct sockaddr* dst, socklen_t dstlen);