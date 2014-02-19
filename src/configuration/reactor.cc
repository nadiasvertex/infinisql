#include "reactor.h"
#include <array>
#include <chrono>
#include <memory>
#include <thread>

namespace infinisql {
namespace configuration {

void
reactor::_respond(const Response& response) {
	std::string data;
	response.SerializeToString(&data);

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
	Request request;
	request.ParseFromString(data);

	_process_request(request);
}

void reactor::_process_request(const Request& r) {
	if (l == nullptr) {
		_respond(_create_response(r.command_id(), Response::STATUS_NOK, Response::REASON_NOT_READY));
	}

	switch(r.cmd()) {
	default:
		_respond(_create_response(r.command_id(), Response::STATUS_NOK, Response::REASON_UNKNOWN_REQUEST));
		break;
	case Request::CMD_ADD_NODE:
		_respond(l->on_add_node());
		break;

	}
}

Response
reactor::_create_response(int command_id, const Response::Status status, const Response::Reason reason) {
	Response r;
	r.set_status(status);
	r.set_reason(reason);
	return r;
}

reactor::reactor(void *socket):socket(socket) {
}


void reactor::process() {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return;
}

} // end namespace configuration
} // end namespace infinisql

