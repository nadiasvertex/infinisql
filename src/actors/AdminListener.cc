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

#include "AdminListener.h"
#include <zmq.h>
#include "../configuration/reactor.h"

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

    while(true) {
        r.process();
    }
}
