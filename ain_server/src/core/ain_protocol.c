#include "ain_config.h"
#include "ain_globals.h"

#include "core/ain_conn.h"
#include "core/ain_protocol.h"
#include "core/ain_sock.h"
#include "core/ain_worker_proc.h"

#define IS_SPACE(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == 0)
#define IS_URI_HOST_END_CHAR(c)                                            \
  (c == '/' || c == '?' || c == ':' || c == '#' || c == '[' || c == ']' || \
   c == ';')
#define IS_EOL(c) (c == '\r' || c == '\n' || c == 0)

ain_result_t ain_proto_parse_uri(ain_uri_t *uri, uint8_t **buf, size_t *size) {
  memset(uri, 0x00, sizeof(ain_uri_t));
  while (IS_SPACE(**buf) && *size) {
    (*buf)++;
    (*size)--;
  }
  if (!*size) return AIN_ERROR;
  uri->scheme = *buf;
  for (; *size && !IS_SPACE(**buf); (*buf)++, (*size)--) {
    if (**buf == ':' && !uri->scheme_size) {
      uri->scheme_size = (*buf - uri->scheme);
      if (uri->scheme_size > 8) return AIN_ERROR;
    } else if (**buf == ':' && uri->host && !uri->port_size &&
               !uri->host_size) {
      uri->host_size = (*buf - uri->scheme) - uri->host;
      uri->port = (*buf - uri->scheme) + 1;
    } else if (**buf == '/' && !uri->path && *size > 0 && *(*buf + 1) == '/') {
      uri->host = (*buf - uri->scheme) + 2;
      (*buf)++;
      (*size)--;
    } else if (**buf == '@') {
      if (uri->host) {
        uri->userinfo = uri->host;
        uri->userinfo_size = (*buf - uri->scheme) - uri->userinfo;
        uri->host = (*buf - uri->scheme) + 1;
        uri->host_size = 0;
        uri->port = 0;
        uri->port_size = 0;
      } else if (uri->path) {
        uri->userinfo = uri->path;
        uri->path = 0;
        uri->userinfo_size = (*buf - uri->scheme) - uri->userinfo;
        uri->host = (*buf - uri->scheme) + 1;
        uri->host_size = 0;
      }
    } else if (**buf == '?') {
      if (!uri->path) uri->path = (*buf - uri->scheme);
      uri->path_size = (*buf - uri->scheme) - uri->path;
      uri->query = (*buf - uri->scheme) + 1;
    } else if (**buf == '#') {
      if (uri->query) {
        uri->query_size = (*buf - uri->scheme) - uri->query;
      } else {
        if (!uri->path) uri->path = (*buf - uri->scheme);
        uri->path_size = (*buf - uri->scheme) - uri->path;
      }
      uri->fragment = (*buf - uri->scheme) + 1;
    } else if (uri->scheme_size && !uri->path && !uri->host) {
      if (uri->port) {
        uri->port_size = (*buf - uri->scheme) - uri->port;
      } else if (uri->host) {
        uri->host_size = (*buf - uri->scheme) - uri->host;
      }
      uri->path = *buf - uri->scheme;
    } else if (!uri->path && uri->host && IS_URI_HOST_END_CHAR(**buf)) {
      if (uri->port) {
        uri->port_size = (*buf - uri->scheme) - uri->port;
      } else if (uri->host) {
        uri->host_size = (*buf - uri->scheme) - uri->host;
      }
      uri->path = (*buf - uri->scheme) + 1;
    }
  }

  if (!uri->path && uri->host) {
    if (uri->port) {
      uri->port_size = (*buf - uri->scheme) - uri->port;
    } else if (uri->host) {
      uri->host_size = (*buf - uri->scheme) - uri->host;
    }
    uri->path = *buf - uri->scheme;
  }

  if (uri->path && !uri->path_size) {
    uri->path_size = (*buf - uri->scheme) - uri->path;
    return AIN_OK;
  }
  if (uri->query && !uri->query_size) {
    uri->query_size = (*buf - uri->scheme) - uri->query;
    return AIN_OK;
  }
  if (uri->fragment && !uri->fragment_size) {
    uri->fragment_size = (*buf - uri->scheme) - uri->fragment;
    return AIN_OK;
  }
  return AIN_ERROR;
}

uint16_t ain_proto_get_default_port(uint8_t *scheme, size_t size) {
  if (AIN_EQUALS(scheme, size, "http"))
    return 80;
  else if (AIN_EQUALS(scheme, size, "https"))
    return 443;
  if (AIN_EQUALS(scheme, size, "sip"))
    return 5060;
  else if (AIN_EQUALS(scheme, size, "sips"))
    return 5061;
  if (AIN_EQUALS(scheme, size, "ainc"))
    return 39480;
  else if (AIN_EQUALS(scheme, size, "ainp"))
    return 39481;
  else
    return 0;
}

extern uint32_t testHTMLGet(ain_conn_t *conn, uint32_t lines);

ain_result_t on_http(ain_conn_t *conn, uint8_t *buf, uint32_t size) {
  if (!size) {
    return AIN_OK;  // connection was closed, cleanup data
  }
  conn->keep_alive = 1;
  if (!conn->write_evt) {
    testHTMLGet(conn, 200);  // dummy write event response
  }
  return AIN_OK;
}

ain_result_t on_ain(ain_conn_t *conn, uint8_t *buf, uint32_t size) {
  if (!size) {
    return AIN_OK;  // connection was closed, cleanup data
  }
  if (AIN_EQUALS(buf, size, "quit\n")) {
    conn->ls->wdata->quit = AIN_WORKER_QUIT_CMD;
    return AIN_ERROR;
  }
  return AIN_ERROR;
}

ain_result_t on_sip(ain_conn_t *conn, uint8_t *buf, uint32_t size) {
  if (!size) {
    return AIN_OK;  // connection was closed, cleanup data
  }

  return AIN_IO_PENDING;
}

ain_result_t on_proto_none(ain_conn_t *conn, uint8_t *buf, uint32_t size) {
  return AIN_ERROR;
}

protocol_handler_t ain_proto_get_type(ain_uri_t *uri) {
  if (AIN_EQUALS(uri->scheme, uri->scheme_size, "http") ||
      AIN_EQUALS(uri->scheme, uri->scheme_size, "https"))
    return on_http;
  else if (AIN_EQUALS(uri->scheme, uri->scheme_size, "sip") ||
           AIN_EQUALS(uri->scheme, uri->scheme_size, "sips"))
    return on_sip;
  else if (AIN_EQUALS(uri->scheme, uri->scheme_size, "ainc") ||
           AIN_EQUALS(uri->scheme, uri->scheme_size, "ainp"))
    return on_ain;

  return on_proto_none;
}
