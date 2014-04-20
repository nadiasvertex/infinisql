/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 
 * InfiniSQL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   AdminListener.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Thu Jan 23 02:27:09 2014
 * 
 * @brief  0mq listener for administrative commands from cluster manager
 */

#include <memory>

#include "AdminListener.h"
#include <zmq.h>
#include "../configuration/reactor.h"
#include "../configuration/listener.h"

namespace infinisql {
	namespace configuration {
		class configuration_listener : public infinisql::configuration::listener {
			bool stop_requested = false;

		public:
			virtual ~configuration_listener() {
			}

			bool is_stop_requested() {
				return stop_requested;
			}

			virtual Response on_add_node(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_remove_node(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_stop_node(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_set_node_type(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_add_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_delete_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_new_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_persist_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_replicate_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_start_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_stop_partition(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_sync_partition(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_add_partition_group(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_start(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_stop(const Request& r) {
				Response response;

				stop_requested = true;
				response.set_status(Response::STATUS_OK);
				response.set_reason(Response::REASON_NONE);

				return response;
			}

			virtual Response on_transaction_log(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_start_transaction_log(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_stop_transaction_log(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_notify_when_log_committed(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_notify_when_log_sent(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_start_transactions(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_finish_and_abort_transactions(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_finish_transactions(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_set_sync(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_stop_sync(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_change_manager(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_change_replica(const Request& r) {
				Response response;
				return response;
			}

			virtual Response on_delete_version(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_get_config(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_node_problem(const Request& r) {
				Response response;
				return response;
			}
			virtual Response on_relayout(const Request& r) {
				Response response;
				return response;
			}
		};
	}
}

AdminListener::AdminListener(Actor::identity_s identity)
    : Actor(identity), zmq_ctx(nullptr), zmq_requestor(nullptr)
{
}

void AdminListener::operator()()
{
    zmq_ctx = zmq_ctx_new();
    if (zmq_ctx==nullptr)
    {
        LOG("can't create 0mq context, exiting");
        exit(1);
    }

    zmq_requestor = zmq_socket(zmq_ctx, ZMQ_REQ);
    if (zmq_requestor==nullptr)
    {
        LOG("can't create 0mq socket, exiting");
        exit(1);
    }

    std::string address { "tcp://localhost:" };
    address.append(identity.zmqhostport);

    if (zmq_connect(zmq_requestor, address.c_str())!=0) {
    	LOG("can't connect 0mq socket to configuration manager, exiting");
    	exit(1);
    }

    infinisql::configuration::reactor r(zmq_requestor);
    auto l = std::make_shared<infinisql::configuration::configuration_listener>();
    r.set_listener(l);

    while(true) {
        r.process();
    }
}
