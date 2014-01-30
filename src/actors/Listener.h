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
 * @file   Listener.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 08:42:37 2014
 * 
 * @brief  On each host, this actor accepts new connections and distributes
 * incoming network traffic from clients to Transaction Agents.
 *
 * There is only one listener per host. It takes a small amount of coding
 * effort to allow for multiple listeners per host. But no workload as yet
 * (benchmarked on 12-core Xeon) has been shown to warrant multiple listeners.
 * It's very possible that larger hosts may benefit from distributing
 * incoming TCP/IP traffic across multiple listeners, and the effort to allow
 * that won't be difficult to implement.
 */

#ifndef INFINISQLLISTENER_H
#define INFINISQLLISTENER_H

#include "Actor.h"

class Listener : public Actor
{
public:
    Listener(Actor::identity_s identity);
    void operator()();
};

#endif // INFINISQLLISTENER_H
