#include "reactor.h"
#include <memory>

namespace infinisql {
namespace configuration {

void
reactor::_request(const std::string &data) {
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, data.size());
	data.copy(static_cast<char*>(zmq_msg_data(&msg)), data.size(), 0);
	zmq_sendmsg(socket, &msg, 0);
}

reactor::reactor(void *socket):socket(socket) {
}


void
reactor::request_status() {
	Request r;
	std::string data;

	r.set_request_type(Request::STATUS);
	r.set_request_match(Request::ALL);

	r.SerializeToString(&data);
	_request(data);
}

void
reactor::request_status_for(nodeid id) {
	Request r;
	std::string data;

	r.set_request_type(Request::STATUS);
	r.set_request_match(Request::SPECIFIC);
	r.set_node_id(id.to_int());

	r.SerializeToString(&data);
	_request(data);
}

} // end namespace configuration
} // end namespace infinisql
