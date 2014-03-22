/*
 * reactor.h
 *
 *  Created on: Jan 23, 2014
 *      Author: Christopher Nelson
 */

#ifndef INFINISQLREACTOR_H_
#define INFINISQLREACTOR_H_

#include <memory>
#include <zmq.h>
#include "nodeid.h"
#include "configuration.pb.h"
#include "listener.h"

namespace infinisql {
namespace configuration {

class reactor {
	// ZMQ socket for sending and receiving messages. This socket
	// must be connected, and set to REQ/REP mode.
	void *socket;

	std::shared_ptr<listener> l;

protected:
	void _respond(const Response& response);
	Response _create_response(int command_id, const Response::Status status, const Response::Reason reason);

	void _check_receive_status();
	void _process_request(const Request& r);
public:
	reactor(void *socket);

	void set_listener(std::shared_ptr<listener> l) {
		this->l = l;
	}

	void process();
};

} // end namespace configuration
} // end namespace infinisql

#endif /* INFINISQLREACTOR_H_ */
