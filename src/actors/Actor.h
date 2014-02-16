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
 * @file   Actor.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 20 22:14:09 2014
 * 
 * @brief  base class for Actors
 */

#ifndef INFINISQLACTOR_H
#define INFINISQLACTOR_H

#include <thread>
#include "../mbox/Mbox.h"
#include "../engine/global.h"
#include "Topology.h"

#define GWBUFSIZE 16777216
#define SERIALIZEDMAXSIZE   1048576

class Mbox;

extern std::atomic<int64_t> *socketAffinity;

class Actor
{
public:
    /** 
     * @brief identifying characteristics for an actor
     */
    struct identity_s
    {
        Message::address_s address;
        int16_t instance;
        Mbox *mbox;
        int epollfd;
        std::string zmqhostport;
        int sockfd;
        MDB_env *env;
        std::string transactionlogpath;
    };

    Actor(identity_s identity);
    void operator()() const;
    virtual ~Actor();

    identity_s identity;
    Message *msgrcv;
    TopologyDistinct myTopology;
};

#endif // INFINISQLACTOR_H
