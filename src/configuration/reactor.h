/*
 * reactor.h
 *
 *  Created on: Jan 23, 2014
 *      Author: Christopher Nelson
 */

#ifndef INFINISQLREACTOR_H_
#define INFINISQLREACTOR_H_

#include <zmq.h>
#include "nodeid.h"
#include "configuration.pb.h"

namespace infinisql {
namespace configuration {

class reactor {
	// ZMQ socket for sending and receiving messages. This socket
	// must be connected, and set to REQ/REP mode.
	void *socket;

protected:
	void _request(const Request& request);
	Request _create_request(const Request::RequestType request_type);
public:
	reactor(void *socket);
	void request_status();
	void request_status_for(nodeid id);
	void request_assignments();
	void request_assignments_for(nodeid id);

	void process();
};

} // end namespace configuration
} // end namespace infinisql

#endif /* INFINISQLREACTOR_H_ */
