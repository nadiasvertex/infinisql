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
	void _request(const std::string &data);
public:
	reactor(void *socket);
	void request_status();
	void request_status_for(nodeid id);

};

} // end namespace configuration
} // end namespace infinisql

#endif /* INFINISQLREACTOR_H_ */
