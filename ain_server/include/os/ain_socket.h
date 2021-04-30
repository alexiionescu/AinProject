#pragma once
ain_result_t ain_sockets_init();
ain_result_t ain_socket_accept(ain_event_t* evt);
ain_result_t ain_socket_onaccept(ain_event_t* evt);
ain_result_t ain_socket_read(ain_event_t* evt);
ain_result_t ain_socket_write(ain_event_t* evt);

