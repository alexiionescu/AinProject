#pragma once

typedef ain_result_t(*protocol_handler_t)(ain_conn_t* conn, 
											uint8_t* buf, size_t size);
struct _ain_proto_data_t {
	protocol_handler_t  proto_handler;
};

struct _ain_uri_t {
	uint8_t*	scheme;
	uint64_t	scheme_size		: 3;
	//next_offset = 1 + max(prev_offset, prev_size)
	uint64_t	userinfo		: 4;
	uint64_t	userinfo_size	: 8;
	uint64_t	host			: 9;	
	uint64_t	host_size		: 8;
	uint64_t	port			: 10;
	uint64_t	port_size		: 3;
	uint64_t	path			: 11;
	uint64_t	path_size		: 12;
	uint64_t	query			: 13;
	uint64_t	query_size		: 14;
	uint64_t	fragment		: 15;
	uint64_t	fragment_size	: 10;
};
ain_result_t ain_proto_parse_uri(ain_uri_t* uri,  
								uint8_t** buf, size_t* size);
uint16_t ain_proto_get_default_port(uint8_t* scheme, size_t size);
protocol_handler_t ain_proto_get_type(ain_uri_t* uri);