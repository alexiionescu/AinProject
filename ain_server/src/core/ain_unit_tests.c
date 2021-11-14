#include "ain_config.h"
#include "ain_globals.h"

#include "os/ain_file.h"
#include "os/ain_io.h"
#include "os/ain_memory.h"
#include "os/ain_process.h"
#include "os/ain_sysinfo.h"
#include "os/ain_worker.h"

#include "core/ain_conn.h"
#include "core/ain_protocol.h"
#include "core/ain_sock.h"
#include "core/ain_worker_proc.h"

void testMemoryPools();
void testProtocol();

void testsUnits() {
  testProtocol();
  // testMemoryPools();
}

void testMemoryPools() {
  ain_mem_pool_t *pool = ain_pool_create(AIN_POOL_SIZE, AIN_MAX_POOLS);

#define TEST_COUNT 1
#define ARRAY_COUNT (AIN_POOL_SIZE / 4)
  for (int t = 0; t < TEST_COUNT; t++) {
    void *parray[ARRAY_COUNT];
    for (size_t i = 0; i < ARRAY_COUNT; i++) {
      size_t req = i / 3 + 1;
      parray[i] = ain_pool_alloc(pool, req);
      memset(parray[i], 1 + i % 255, req * AIN_PAGE_SIZE);
    }
    for (int i = 0; i < ARRAY_COUNT; i++) ain_pool_free(pool, parray[i]);
  }
#undef TEST_COUNT
#undef ARRAY_COUNT

#define TEST_COUNT 1
#define ARRAY_COUNT (AIN_POOL_SIZE * 4)
  for (int t = 0; t < TEST_COUNT; t++) {
    void *parray[ARRAY_COUNT];
    for (size_t i = 0; i < ARRAY_COUNT; i++) {
      size_t req = 1;
      parray[i] = ain_pool_alloc(pool, req);
      memset(parray[i], 1 + i % 255, req * AIN_PAGE_SIZE);
    }
    for (int i = 0; i < ARRAY_COUNT; i++) ain_pool_free(pool, parray[i]);
  }
#undef TEST_COUNT
#undef ARRAY_COUNT

  ain_pool_destroy(pool);

  pool = ain_pool_create(0, 0);
#define TEST_COUNT 1
#define ARRAY_COUNT (4)
  for (int t = 0; t < TEST_COUNT; t++) {
    void *parray[ARRAY_COUNT];
    for (size_t i = 0; i < ARRAY_COUNT; i++) {
      size_t req = i + 1;
      parray[i] = ain_pool_alloc(pool, req);
      memset(parray[i], 1 + i % 255, req * AIN_PAGE_SIZE);
    }
    for (int i = 0; i < ARRAY_COUNT; i++) ain_pool_free(pool, parray[i]);
  }
#undef TEST_COUNT
#undef ARRAY_COUNT
  ain_pool_destroy(pool);
}

uint8_t _uri_tests[] = {
    "   ain:dsds:rere:ttret\n"
    "   http://localhost"
    "   http://192.168.0.014:43435"
    "   "
    "https://username:password@doamin.com:2021/a/b/test/"
    "index.html?q1=232&q2=343#sub-chapter\r"
    "   https://google.com:20213/a/b/reewerwe/d.html?a1=232&a3=343\r\n"
    "   https://google.com/a/b/reewerwe/d.html\r\n"
    "   sip:dsds@192.168.168.140\0"
    "   sip:dsds@192.168.168.140:3434"
    "   sip:dsds@192.168.168.140;transport=udp\t"
    "   sip:dsds@192.168.168.140:3434;transport=udp?asa"};

void testProtocol() {
  ain_mem_pool_t *pool = ain_pool_create(AIN_POOL_SIZE, AIN_MAX_POOLS);
#define TEST_COUNT 1
#define ARRAY_COUNT (AIN_POOL_SIZE)
  for (int t = 0; t < TEST_COUNT; t++) {
    ain_event_t *parray[ARRAY_COUNT];
    for (size_t i = 0; i < ARRAY_COUNT; i++) {
      size_t req = 1;
      parray[i] = ain_pool_alloc(pool, req);
      parray[i]->buf_size = sizeof(_uri_tests);
      uint8_t *buf = EVENT_BUFFER(parray[i]);
      memcpy(buf, _uri_tests, parray[i]->buf_size);
      size_t s = sizeof(_uri_tests);
      ain_uri_t uri;
      while (AIN_OK == ain_proto_parse_uri(&uri, &buf, &s)) {
        if (i == 0 && t == 0) {
          printf("<%.*s>://<%.*s>@<%.*s>:<%.*s> <%.*s>?<%.*s>#<%.*s>\n",
                 (int)uri.scheme_size, uri.scheme, (int)uri.userinfo_size,
                 uri.scheme + uri.userinfo, (int)uri.host_size,
                 uri.scheme + uri.host, (int)uri.port_size,
                 uri.scheme + uri.port, (int)uri.path_size,
                 uri.scheme + uri.path, (int)uri.query_size,
                 uri.scheme + uri.query, (int)uri.fragment_size,
                 uri.scheme + uri.fragment);
        }
      }
    }
    for (int i = 0; i < ARRAY_COUNT; i++) ain_pool_free(pool, parray[i]);
  }
#undef TEST_COUNT
#undef ARRAY_COUNT
  ain_pool_destroy(pool);
}

const char _html_response_antet[] =
    "HTTP/1.1 200 OK\r\n"
    "Server: ain/0.0.1\r\n"
    "Date: Thu, 29 Apr 2021 13:44:07 GMT\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %u\r\n"
    "Last-Modified: Tue, 13 Apr 2021 01:25:24 GMT\r\n"
    // "Connection: close\r\n"
    "Connection: keep-alive\r\n"
    "ETag: \"6074f304-6567\"\r\n"
    "Accept-Ranges: bytes\r\n\r\n";

const char _html_doc_start[] =
    "<!DOCTYPE html>\r\n"
    "<html><body>\r\n";

const char _html_doc_content[] =
    "123456789 123456789 123456789 123456789 123456789 123456789<BR>\r\n";

const char _html_doc_end[] = "</body></html>\r\n";

uint32_t testHTMLGet(ain_conn_t *conn, uint32_t lines) {
  uint32_t size = sizeof(_html_doc_start) + sizeof(_html_doc_end) - 2;
  size += lines * (sizeof(_html_doc_content) - 1);

  uint32_t req = size + sizeof(_html_response_antet) + 300;
  AIN_BYTES_TO_PAGES(req);
  ain_event_t *evt =
      ain_event_alloc(conn->ls->wdata->pool, sizeof(ain_conn_t *), req, 1);
  *(ain_conn_t **)EVENT_DATA(evt) = conn;

  uint8_t *buf = EVENT_BUFFER(evt);
  evt->buf_size = 0;

  char html_response_antet[sizeof(_html_response_antet) + 10];
  int written = snprintf(html_response_antet, sizeof(html_response_antet),
                         _html_response_antet, size);

  memcpy(buf, html_response_antet, written);
  buf += written;
  evt->buf_size += written;

  written = sizeof(_html_doc_start) - 1;
  memcpy(buf, _html_doc_start, written);
  buf += written;
  evt->buf_size += written;

  for (uint32_t l = 0; l < lines; l++) {
    written = sizeof(_html_doc_content) - 1;
    memcpy(buf, _html_doc_content, written);
    buf += written;
    evt->buf_size += written;
  }

  written = sizeof(_html_doc_end) - 1;
  memcpy(buf, _html_doc_end, written);
  buf += written;
  evt->buf_size += written;

  evt->iov[0].iov_len = evt->buf_size;
  conn->write_evt = evt;
  ain_register_io_events(conn->ls->wdata->ioqe, &conn->write_evt, 1);
  // printf("[%d] testHTMLGet %zu bytes\n", conn->fd, evt->buf_size);
  return 0;
}
