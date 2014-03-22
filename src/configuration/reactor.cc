#include "reactor.h"
#include <array>
#include <chrono>
#include <memory>
#include <thread>

namespace infinisql {
namespace configuration {

void reactor::_respond(const Response& response) {
	std::string data;
	response.SerializeToString(&data);

	zmq_msg_t msg;
	zmq_msg_init_size(&msg, data.size());
	data.copy(static_cast<char*>(zmq_msg_data(&msg)), data.size(), 0);
	zmq_sendmsg(socket, &msg, 0);
}

void reactor::_check_receive_status() {
	std::array<zmq_pollitem_t, 1> poll_items { { { socket, 0, ZMQ_POLLIN, 0 } } };

	if (zmq_poll(poll_items.data(), poll_items.size(), 10000) == 0) {
		return;
	}

	zmq_msg_t msg;
	zmq_msg_init(&msg);
	zmq_recvmsg(socket, &msg, 0);

	std::string data(static_cast<char*>(zmq_msg_data(&msg)),
			zmq_msg_size(&msg));
	Request request;
	request.ParseFromString(data);

	_process_request(request);
}

void reactor::_process_request(const Request& r) {
	if (l == nullptr) {
		_respond(
				_create_response(r.command_id(), Response::STATUS_NOK,
						Response::REASON_NOT_READY));
	}

	switch (r.cmd()) {
	default:
		_respond(
				_create_response(r.command_id(), Response::STATUS_NOK,
						Response::REASON_UNKNOWN_REQUEST));
		break;
	case Request::CMD_ADD_NODE:
		_respond(l->on_add_node(r));
		break;
	case Request::CMD_ADD_PARTITION:
		_respond(l->on_add_partition(r));
		break;
	case Request::CMD_ADD_PARTITION_GROUP:
		_respond(l->on_add_partition_group(r));
		break;
	case Request::CMD_CHANGE_MANAGER:
		_respond(l->on_change_manager(r));
		break;
	case Request::CMD_CHANGE_REPLICA:
		_respond(l->on_change_replica(r));
		break;
	case Request::CMD_DELETE_PARTITION:
		_respond(l->on_delete_partition(r));
		break;
	case Request::CMD_DELETE_VERSION:
		_respond(l->on_delete_version(r));
		break;
	case Request::CMD_FINISH_AND_ABORT_TRANSACTIONS:
		_respond(l->on_finish_and_abort_transactions(r));
		break;
	case Request::CMD_FINISH_TRANSACTIONS:
		_respond(l->on_finish_transactions(r));
		break;
	case Request::CMD_GET_CONFIG:
		_respond(l->on_get_config(r));
		break;
	case Request::CMD_NEW_PARTITION:
		_respond(l->on_new_partition(r));
		break;
	case Request::CMD_NODE_PROBLEM:
		_respond(l->on_node_problem(r));
		break;
	case Request::CMD_NOTIFY_WHEN_LOG_COMMITTED:
		_respond(l->on_notify_when_log_committed(r));
		break;
	case Request::CMD_NOTIFY_WHEN_LOG_SENT:
		_respond(l->on_notify_when_log_sent(r));
		break;
	case Request::CMD_PERSIST_PARTITION:
		_respond(l->on_persist_partition(r));
		break;
	case Request::CMD_RELAYOUT:
		_respond(l->on_relayout(r));
		break;
	case Request::CMD_REMOVE_NODE:
		_respond(l->on_remove_node(r));
		break;
	case Request::CMD_REPLICATE_PARTITION:
		_respond(l->on_replicate_partition(r));
		break;
	case Request::CMD_SET_NODE_TYPE:
		_respond(l->on_set_node_type(r));
		break;
	case Request::CMD_SET_SYNC:
		_respond(l->on_set_sync(r));
		break;
	case Request::CMD_START:
		_respond(l->on_start(r));
		break;
	case Request::CMD_START_PARTITION:
		_respond(l->on_start_partition(r));
		break;
	case Request::CMD_START_TRANSACTION_LOG:
		_respond(l->on_start_transaction_log(r));
		break;
	case Request::CMD_START_TRANSACTIONS:
		_respond(l->on_start_transactions(r));
		break;
	case Request::CMD_STOP:
		_respond(l->on_stop(r));
		break;
	case Request::CMD_STOP_TRANSACTION_LOG:
		_respond(l->on_stop_transaction_log(r));
		break;
	case Request::CMD_STOP_NODE:
		_respond(l->on_stop_node(r));
		break;
	case Request::CMD_STOP_PARTITION:
		_respond(l->on_stop_partition(r));
		break;
	case Request::CMD_STOP_SYNC:
		_respond(l->on_stop_sync(r));
		break;
	case Request::CMD_SYNC_PARTITION:
		_respond(l->on_sync_partition(r));
		break;
	}
}

Response reactor::_create_response(int command_id,
		const Response::Status status, const Response::Reason reason) {
	Response r;
	r.set_status(status);
	r.set_reason(reason);
	return r;
}

reactor::reactor(void *socket) :
		socket(socket) {
}

void reactor::process() {
	_check_receive_status();
}

} // end namespace configuration
} // end namespace infinisql

