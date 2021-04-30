#include "ain_config.h"

#define _AIN_USE_SOCKETS_
#include "ain_globals.h"
#include "os/ain_socket.h"
#include "os/ain_io.h"
#include "core/ain_event.h"
#include "core/ain_conn.h"
#include "core/ain_sock.h"

#pragma comment(lib, "ws2_32.lib")

LPFN_ACCEPTEX              ain_acceptex;
LPFN_GETACCEPTEXSOCKADDRS  ain_getacceptexsockaddrs;
LPFN_TRANSMITFILE          ain_transmitfile;
LPFN_TRANSMITPACKETS       ain_transmitpackets;
LPFN_CONNECTEX             ain_connectex;
LPFN_DISCONNECTEX          ain_disconnectex;
static GUID ax_guid = WSAID_ACCEPTEX;
static GUID as_guid = WSAID_GETACCEPTEXSOCKADDRS;
static GUID tf_guid = WSAID_TRANSMITFILE;
static GUID tp_guid = WSAID_TRANSMITPACKETS;
static GUID cx_guid = WSAID_CONNECTEX;
static GUID dx_guid = WSAID_DISCONNECTEX;

ain_result_t ain_sockets_init()
{
    WSADATA       wsd;
    AIN_ASSERT_ERR(0 == WSAStartup(MAKEWORD(2, 2), &wsd));

    SOCKET s = ain_socket_create(AF_INET, SOCK_STREAM, 0);
    AIN_ASSERT_ERR(s != INVALID_SOCKET);

    DWORD bytes;
    AIN_ASSERT_ERR(0 == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, 
                            &ax_guid, sizeof(GUID),
                            &ain_acceptex, sizeof(LPFN_ACCEPTEX), 
                            &bytes, NULL, NULL));
    AIN_ASSERT_ERR(0 == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                            &as_guid, sizeof(GUID),
                            &ain_getacceptexsockaddrs, sizeof(LPFN_GETACCEPTEXSOCKADDRS),
                            &bytes, NULL, NULL));
    AIN_ASSERT_ERR(0 == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                            &tf_guid, sizeof(GUID),
                            &ain_transmitfile, sizeof(LPFN_TRANSMITFILE),
                            &bytes, NULL, NULL));
    AIN_ASSERT_ERR(0 == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                            &tp_guid, sizeof(GUID),
                            &ain_transmitpackets, sizeof(LPFN_TRANSMITPACKETS),
                            &bytes, NULL, NULL));
    AIN_ASSERT_ERR(0 == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                            &cx_guid, sizeof(GUID),
                            &ain_connectex, sizeof(LPFN_CONNECTEX),
                            &bytes, NULL, NULL));
    AIN_ASSERT_ERR(0 == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                            &dx_guid, sizeof(GUID),
                            &ain_disconnectex, sizeof(LPFN_DISCONNECTEX),
                            &bytes, NULL, NULL));
    return AIN_OK;
}


ain_result_t ain_socket_accept(ain_event_t* evt)
{
    ain_conn_t* conn = *(ain_conn_t**)EVENT_DATA(evt);
    DWORD addrSize = conn->ls->sockaddrlen + 16;
    DWORD dwReceivedBytes = 0;
    evt->handler = ain_socket_onaccept;
    memset(evt, 0, sizeof(OVERLAPPED));
    if (ain_acceptex(conn->ls->fd, conn->fd, EVENT_BUFFER(evt), 0,
                    addrSize, addrSize, &dwReceivedBytes, (LPOVERLAPPED)evt))
    {
        evt->handler(evt);
        return AIN_OK;
    }
    AIN_ASSERT_ERR(AIN_ERRNO == ERROR_IO_PENDING);
    return AIN_OK;
}


ain_result_t ain_socket_onaccept(ain_event_t* evt)
{
    ain_conn_t* conn = *(ain_conn_t**)EVENT_DATA(evt);
    if (evt->error) {
        ain_conn_close(conn);
        return AIN_ERROR;
    }
    uint8_t* buf = EVENT_BUFFER(evt);
    DWORD addrSize = conn->ls->sockaddrlen + 16;
    if(-1 == setsockopt(conn->fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                        (char*)&conn->ls->fd, sizeof(ain_fd_t)))
    {
        ain_conn_close(conn);
        return AIN_ERROR;
    }

    struct sockaddr* lsaddr, *rsaddr;
    socklen_t lsaddrlen, rsaddrlen;
    ain_getacceptexsockaddrs(buf, 
        0,
        addrSize, addrSize,
        &lsaddr, &lsaddrlen,
        &rsaddr, &rsaddrlen);
    conn->connected = 1;
    return ain_socket_read(evt);
}

ain_result_t ain_socket_read(ain_event_t* evt)
{
    ain_conn_t* conn = *(ain_conn_t**)EVENT_DATA(evt);
    DWORD dwReceivedBytes = 0;
    WSABUF DataBuf;
    DataBuf.buf = (CHAR*)EVENT_BUFFER(evt);
    DataBuf.len = (ULONG)EVENT_BUFFER_SIZE(evt);
    evt->handler = ain_conn_onread;
    memset(evt, 0, sizeof(OVERLAPPED));
    DWORD Flags = 0;
    evt->buf_size = 0;
    if (0 == WSARecv(evt->fd, &DataBuf, 1, &dwReceivedBytes, &Flags, 
                        (LPOVERLAPPED)evt,NULL))
    {
        evt->error = 0;
        evt->buf_size = dwReceivedBytes;
        evt->handler(evt);
        return AIN_OK;
    }
    if (WSA_IO_PENDING == WSAGetLastError())
        return  AIN_OK;

    ain_conn_close(conn);
    return AIN_ERROR;
}

ain_result_t ain_socket_write(ain_event_t* evt)
{
    ain_conn_t* conn = *(ain_conn_t**)EVENT_DATA(evt);
    DWORD dwSentBytes = 0;
    WSABUF DataBuf;
    DataBuf.buf = (CHAR*)EVENT_BUFFER(evt);
    DataBuf.len = evt->buf_size;
    DWORD Flags = 0;
    evt->handler = ain_conn_onwrite;
    memset(evt, 0, sizeof(OVERLAPPED));
    if (0 == WSASend(evt->fd, &DataBuf, 1, &dwSentBytes, Flags,
                     (LPOVERLAPPED)evt, NULL))
    {
        evt->error = 0;
        evt->buf_size = dwSentBytes;
        evt->handler(evt);
        return AIN_OK;
    }
    if (WSA_IO_PENDING == WSAGetLastError())
        return  AIN_OK;

    ain_conn_close(conn);
    return AIN_ERROR;
}
