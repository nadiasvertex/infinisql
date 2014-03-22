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
 * @file   Topology.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Wed Jan 22 09:05:35 2014
 * 
 * @brief Topology class has all of the actors, their types, and dynamic
 * configuration values. Each actor maintains a Topology object which gives
 * it a common view for the whole node and, as necessary, the whole cluster.
 */

#ifndef INFINISQLTOPOLOGY_H
#define INFINISQLTOPOLOGY_H

#include "../engine/global.h"
#include "../engine/UserSchemaDb.h"

class Mbox;

class Topology
{
public:
    Topology();
    virtual ~Topology();

    int16_t nodeid;
    /**
     * @todo pattern for adding a localTransactionAgent:
     * 1) after adding it, and releasing the nodeTopologyMutex
     * 2) increment localTransactionAgentsVersion
     *
     * that tells Listener to copy nodeTopology--Listener, doesn't
     * read it's Mbox, so it needs to know about that change in
     * Topology every time a new socket is accept()ed
     */
    std::vector<Mbox *> localTransactionAgents;
    std::vector<Mbox *> actoridToMboxes;
    // nodeidToIbGateway[nodeid]={"host", "port"};
    std::map< int16_t, std::pair<std::string, std::string> > nodeidToIbGateway;
    UserSchemaDb userSchemaDb;
};

/** 
 * @brief TopologyDistinct is a specific Actor's Topology
 */
class TopologyDistinct : public Topology
{
public:
    TopologyDistinct();

    bool update();

    int topologyVersion;
    Mbox *obGateway;
    MessageBatch *obBatch;
    Message::address_s userSchemaManager;
};

extern Topology nodeTopology;
extern std::mutex nodeTopologyMutex;
extern std::atomic<int> nodeTopologyVersion;

#endif // INFINISQLTOPOLOGY_H
