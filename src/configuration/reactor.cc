#include "reactor.h"
#include <array>
#include <chrono>
#include <memory>
#include <thread>

namespace infinisql {
namespace configuration {

void
reactor::_request(const Request& request) {
	std::string data;
	request.SerializeToString(&data);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, data.size());
	data.copy(static_cast<char*>(zmq_msg_data(&msg)), data.size(), 0);
	zmq_sendmsg(socket, &msg, 0);
}

void reactor::_check_receive_status() {
	std::array<zmq_pollitem_t, 1> poll_items { { { socket, 0, ZMQ_POLLIN, 0 } } };

	if (zmq_poll(poll_items.data(), poll_items.size(), 10000)==0) {
		return;
	}

	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(socket, &msg, 0);

	std::string data(static_cast<char*>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
	Response response;
	response.ParseFromString(data);
}

Request
reactor::_create_request(const Request::RequestType request_type) {
	Request r;
	r.set_request_type(request_type);
	return r;
}

reactor::reactor(void *socket):socket(socket) {
}

void
reactor::request_status() {
	auto r = _create_request(Request::STATUS);
	r.set_request_match(Request::ALL);
	_request(r);
}

void
reactor::request_status_for(nodeid id) {
	auto r = _create_request(Request::STATUS);
	r.set_request_match(Request::SPECIFIC);
	r.set_node_id(id.to_int());
	_request(r);}

void reactor::request_assignments() {
	auto r = _create_request(Request::ASSIGNMENT);
	r.set_request_match(Request::ALL);
	_request(r);
}

void reactor::request_assignments_for(nodeid id) {
	auto r = _create_request(Request::ASSIGNMENT);
	r.set_request_match(Request::SPECIFIC);
	r.set_node_id(id.to_int());
	_request(r);
}


void reactor::process() {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return;
}

} // end namespace configuration
} // end namespace infinisql

