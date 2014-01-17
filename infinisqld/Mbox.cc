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
 * @file   Mbox.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:31:24 2013
 * 
 * @brief  Mbox* classes perform inter-actor messaging. Mbox is a lockfree
 * multi producer, single consumer queue.
 */

#include "gch.h"
#include "Mbox.h"
#include <time.h>
#line 33 "Mbox.cc"

/** lockfree producer adapted from:
 * http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html */

Mbox::Mbox() : counter(8888)
{
    firstMsg = new class Message();
    firstMsg->messageStruct.payloadtype = PAYLOADNONE;
    firstMsg->nextmsg = getInt128FromPointer(NULL, 5000);
    currentMsg = firstMsg;
    lastMsg = firstMsg;
    myLastMsg = firstMsg;

    head = getInt128FromPointer(firstMsg, 10000);
    tail.store(head);
    current.store(head);
    mytail.store(head);

    pthread_mutexattr_t attr;
    attr.__align = PTHREAD_MUTEX_ADAPTIVE_NP;
    pthread_mutex_init(&mutexLast, &attr);
}

Mbox::~Mbox()
{
    delete firstMsg;
}

// return *message, NULL if timeout or nothing
// timeout: -1, wait forever (well, uncertain effect)
// timeout: 0, do not wait
// timeout: >0, microseconds to wait
//size_t Mbox::receive(class Message *msg, int timeout)
class Message *Mbox::receive(int timeout)
{
    __int128 mynext;

    while (1)
    {
        mynext = getPtr(current)->nextmsg;

        if (getPtr(mynext)==NULL)
        {
            switch (timeout)
            {
            case -1:
                break;

            case 0:
                return NULL;
                break;

            default:
                struct timespec ts = {0, timeout * 1000};
                nanosleep(&ts, NULL);
                return NULL;
            }
        }
        else
        {
            if (getPtr(current)==getPtr(mynext))
            {
                printf("%s %i WTF found it i guess count current %lu next %lu\n",
                       __FILE__, __LINE__, getCount(current), getCount(mynext));
            }

            delete getPtr(current);
            current = mynext;
            return getPtr(current);      
        }
    }
}

__int128 Mbox::getInt128FromPointer(class Message *ptr, uint64_t count)
{
    __int128 i128;
    memcpy(&i128, &ptr, sizeof(ptr));
    memcpy((uint64_t *)&i128+1, &count, sizeof(count));

    return i128;
}

Message *Mbox::getPtr(__int128 i128)
{
    return reinterpret_cast<Message *>(i128);
}

uint64_t Mbox::getCount(__int128 i128)
{
    return *((uint64_t *)&i128+1);
}

MboxProducer::MboxProducer() : mbox(NULL), obBatchMsg(NULL)
{
}

MboxProducer::MboxProducer(class Mbox *mboxarg, int16_t nodeidarg) :
    mbox(mboxarg), nodeid(nodeidarg), obBatchMsg(NULL)
{
}

MboxProducer::~MboxProducer()
{
}

void MboxProducer::sendMsg(class Message &msgsnd)
{
    class Message *msgptr;
    if (nodeid != msgsnd.messageStruct.destAddr.nodeid)
    { // must be sending to obgw then, so serialize here
        if (obBatchMsg==NULL)
        {
            obBatchMsg=new class MessageBatchSerialized(nodeid);
        }
        obBatchMsg->msgbatch[obBatchMsg->nmsgs++]=
            {msgsnd.messageStruct.destAddr.nodeid, msgsnd.sermsg()};
        delete &msgsnd;
        if (obBatchMsg->nmsgs==OBGWMSGBATCHSIZE)
        {
            mboxes->sendObBatch();
        }
        return;
    }
    else
    {
        msgptr=&msgsnd;
    }
    class Message &msg=*msgptr;
  
    msg.nextmsg = Mbox::getInt128FromPointer(NULL, 5555);

    __int128 mytail;
    __int128 mynext;

    while (1)
    {
        mytail = mbox->tail;
        mynext = Mbox::getPtr(mytail)->nextmsg;

        if (mytail == mbox->tail)
        {
            if (Mbox::getPtr(mynext) == NULL)
            {
                if (Mbox::getPtr(mytail)->nextmsg
                	 .compare_exchange_strong(mynext,
                			 Mbox::getInt128FromPointer(&msg,
                					 mbox->counter.fetch_add(1))))
                {
                    break;
                }
            }
            else
            {
                // CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
                mbox->tail.compare_exchange_strong(mytail,
                		Mbox::getInt128FromPointer(Mbox::getPtr(mynext),
                				mbox->counter.fetch_add(1)));
            }
        }
    }

    mbox->tail.compare_exchange_strong(mytail,
    		Mbox::getInt128FromPointer(&msg, mbox->counter.fetch_add(1)));
}

Mboxes::Mboxes() : Mboxes(0)
{
}

Mboxes::Mboxes(int64_t nodeidarg) : nodeid(nodeidarg),
                                    topologyMgrPtr(NULL),
                                    userSchemaMgrLocation (),
                                    deadlockMgrLocation (),
                                    listenerPtr(NULL),
                                    obGatewayPtr (NULL)
{
    topologyMgr.mbox = NULL;
    userSchemaMgr.mbox = NULL;
    deadlockMgr.mbox = NULL;
    listener.mbox = NULL;
    ibGateway.mbox = NULL;
    obGateway.mbox = NULL;
}

Mboxes::~Mboxes()
{
}

/** Updates the set of MboxProducers based on updated Topology. Should be called
 * by an actor after it receives a message from TopologyMgr that a change in
 * nodeTopology has happened. Should be passed a Topology local to the
 * actor. */
void Mboxes::update(class Topology &top)
{
    update(top, 0);
}

void Mboxes::update(class Topology &top, int64_t myActorid)
{
    class Topology *newTop = new class Topology();
    std::swap(*newTop, top);
    delete newTop;
    pthread_mutex_lock(&nodeTopologyMutex);
    top = nodeTopology;
    pthread_mutex_unlock(&nodeTopologyMutex);

    // new, so delete the previous logic when switched over
    // local
    actoridToProducers.resize(top.actorList.size(), NULL);
    transactionAgentPtrs.resize(top.numtransactionagents, NULL);
    enginePtrs.resize(top.numengines, NULL);

    for (size_t n=0; n < top.actorList.size(); n++)
    {
        if (actoridToProducers[n]==NULL)
        {
            if (top.actorList[n].type != ACTOR_NONE)
            {
                actoridToProducers[n] =
                    new class MboxProducer(top.actorList[n].mbox,top.nodeid);
                actoridToProducers[n]->mboxes=this;
            }

            switch (top.actorList[n].type)
            {
            case ACTOR_TRANSACTIONAGENT:
                transactionAgentPtrs[top.actorList[n].instance] =
                    actoridToProducers[n];
                break;

            case ACTOR_ENGINE:
                enginePtrs[top.actorList[n].instance] = actoridToProducers[n];
                break;

            case ACTOR_TOPOLOGYMGR:
                topologyMgrPtr = actoridToProducers[n];
                break;

            case ACTOR_LISTENER:
                listenerPtr = actoridToProducers[n];
                break;

            case ACTOR_USERSCHEMAMGR:
            {
                userSchemaMgrLocation.address.nodeid = top.nodeid;
                userSchemaMgrLocation.address.actorid = n;
                userSchemaMgrLocation.destmbox = actoridToProducers[n];
            }
            break;

            case ACTOR_DEADLOCKMGR:
            {
                deadlockMgrLocation.address.nodeid = top.nodeid;
                deadlockMgrLocation.address.actorid = n;
                deadlockMgrLocation.destmbox = actoridToProducers[n];
            }
            break;

            case ACTOR_IBGATEWAY:
                break;

            case ACTOR_OBGATEWAY:
                if ((int64_t)(myActorid % top.numobgateways) ==
                    top.actorList[n].instance)
                {
                    obGatewayPtr = actoridToProducers[n];
                }
                break;
          
            case ACTOR_NONE:
                break;

            default:
                printf("%s %i unhandled actor type: %i\n", __FILE__, __LINE__,
                       top.actorList[n].type);
            }
        }
    }

    // global
    location_s emptyLocation = {};
    partitionToProducers.resize(top.numpartitions, emptyLocation);

    for (size_t n=0; n < top.partitionListThisReplica.size(); n++)
    {
        if (partitionToProducers[n].address.nodeid==0)
        {
            partitionToProducers[n].address =
                top.partitionListThisReplica[n].address;

            if (top.nodeid==partitionToProducers[n].address.nodeid)
            {
                partitionToProducers[n].destmbox =
                    actoridToProducers[partitionToProducers[n].address.actorid];
            }
            else
            {
                partitionToProducers[n].destmbox = obGatewayPtr;
            }
        }
    }

    vector< vector<int> > aa = top.allActors;
    allActors.swap(aa);
    boost::unordered_map< int16_t, vector<int> > aatr = top.allActorsThisReplica;
    allActorsThisReplica.swap(aatr);

    if (top.userSchemaMgrNode && (top.userSchemaMgrNode != top.nodeid))
    {
        userSchemaMgrLocation.address = {top.userSchemaMgrNode, 3};
    }

    if (top.deadlockMgrNode && (top.deadlockMgrNode != top.nodeid))
    {
        deadlockMgrLocation.address = {top.deadlockMgrNode, 2};
    }
}

/** Sends message by global actor address */
void Mboxes::toActor(const Topology::addressStruct &source,
                     const Topology::addressStruct &dest, class Message &msg)
{
    msg.setEnvelope(source, dest, msg);

    if (nodeid==dest.nodeid)
    {
        actoridToProducers[dest.actorid]->sendMsg(msg);
    }
    else
    {
        obGatewayPtr->sendMsg(msg);
    }
}

void Mboxes::toUserSchemaMgr(const Topology::addressStruct &source,
                             class Message &msg)
{
    toActor(source, userSchemaMgrLocation.address, msg);
}

void Mboxes::toDeadlockMgr(const Topology::addressStruct &source,
                           class Message &msg)
{
    toActor(source, deadlockMgrLocation.address, msg);
}

void Mboxes::toPartition(const Topology::addressStruct &source,
                         int64_t partitionid, class Message &msg)
{
    if (partitionid < 0 || (size_t)partitionid > partitionToProducers.size()-1)
    {
        printf("%s %i anomaly %li %lu\n", __FILE__, __LINE__, partitionid,
               (unsigned long)partitionToProducers.size());
        return;
    }

    location_s &destLocation = partitionToProducers[partitionid];
    msg.setEnvelope(source, destLocation.address, msg);
    destLocation.destmbox->sendMsg(msg);
}

int64_t Mboxes::toAllOfType(actortypes_e type,
                            const Topology::addressStruct &source,
                            class Message &msg)
{
    int64_t tally = 0;

    for (int16_t n=1; n < (int16_t)allActors.size(); n++)
    {
        for (int16_t m=FIRSTACTORID; m < (int16_t)allActors[n].size(); m++)
        {
            if (allActors[n][m] == (int)type)
            {
                switch (msg.messageStruct.payloadtype)
                {
                case PAYLOADMESSAGE:
                {
                    class Message *nmsg = new class Message;
                    *nmsg = *((class Message *)&msg);
                    toActor(source, {n, m}, *nmsg);
                }
                break;

                case PAYLOADSOCKET:
                {
                    class MessageSocket *nmsg = new class MessageSocket;
                    *nmsg = *((class MessageSocket *)&msg);
                    toActor(source, {n, m}, *nmsg);
                }
                break;

                case PAYLOADUSERSCHEMA:
                {
                    class MessageUserSchema *nmsg = new class MessageUserSchema;
                    *nmsg = *((class MessageUserSchema *)&msg);
                    toActor(source, {n, m}, *nmsg);
                }
                break;

                case PAYLOADDEADLOCK:
                {
                    class MessageDeadlock *nmsg = new class MessageDeadlock;
                    *nmsg = *((class MessageDeadlock *)&msg);
                    toActor(source, {n, m}, *nmsg);
                }
                break;

                case PAYLOADSUBTRANSACTION:
                {
                    class MessageSubtransactionCmd *nmsg =
                        new class MessageSubtransactionCmd;
                    *nmsg = *((class MessageSubtransactionCmd *)&msg);
                    toActor(source, {n, m}, *nmsg);
                }
                break;

                case PAYLOADCOMMITROLLBACK:
                {
                    class MessageCommitRollback *nmsg =
                        new class MessageCommitRollback;
                    *nmsg = *((class MessageCommitRollback *)&msg);
                    toActor(source, {n, m}, *nmsg);
                }
                break;

                default:
                    printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                           msg.messageStruct.payloadtype);
                }

                tally++;
            }
        }
    }

    return tally;
}

int64_t Mboxes::toAllOfTypeThisReplica(actortypes_e type,
                                       const Topology::addressStruct &source,
                                       class Message &msg)
{
    int64_t tally = 0;

    boost::unordered_map< int16_t, vector<int> >::iterator it;

    for (it = allActorsThisReplica.begin(); it != allActorsThisReplica.end();
         ++it)
    {
        for (int16_t m=FIRSTACTORID; m <
                 (int16_t)allActorsThisReplica[it->first].size(); m++)
        {
            if (allActorsThisReplica[it->first][m] == (int)type)
            {
                printf("%s %i n m %i %i allActors %i allActorsThisReplica %i\n",
                       __FILE__, __LINE__, it->first, m, allActors[it->first][m],
                       allActorsThisReplica[it->first][m]);

                switch (msg.messageStruct.payloadtype)
                {
                case PAYLOADMESSAGE:
                {
                    class Message *nmsg = new class Message;
                    *nmsg = *((class Message *)&msg);
                    toActor(source, {it->first, m}, *nmsg);
                }
                break;

                case PAYLOADSOCKET:
                {
                    class MessageSocket *nmsg = new class MessageSocket;
                    *nmsg = *((class MessageSocket *)&msg);
                    toActor(source, {it->first, m}, *nmsg);
                }
                break;

                case PAYLOADUSERSCHEMA:
                {
                    class MessageUserSchema *nmsg = new class MessageUserSchema;
                    *nmsg = *((class MessageUserSchema *)&msg);
                    toActor(source, {it->first, m}, *nmsg);
                }
                break;

                case PAYLOADDEADLOCK:
                {
                    class MessageDeadlock *nmsg = new class MessageDeadlock;
                    *nmsg = *((class MessageDeadlock *)&msg);
                    toActor(source, {it->first, m}, *nmsg);
                }
                break;

                case PAYLOADSUBTRANSACTION:
                {
                    class MessageSubtransactionCmd *nmsg =
                        new class MessageSubtransactionCmd;
                    *nmsg = *((class MessageSubtransactionCmd *)&msg);
                    toActor(source, {it->first, m}, *nmsg);
                }
                break;

                case PAYLOADCOMMITROLLBACK:
                {
                    class MessageCommitRollback *nmsg =
                        new class MessageCommitRollback;
                    *nmsg = *((class MessageCommitRollback *)&msg);
                    toActor(source, {it->first, m}, *nmsg);
                }
                break;

                default:
                    printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                           msg.messageStruct.payloadtype);
                }

                tally++;
            }
        }
    }

    return tally;
}

void Mboxes::sendObBatch()
{
    if (obGatewayPtr != NULL)
    {
        if (obGatewayPtr->obBatchMsg != NULL)
        {
            obGatewayPtr->sendMsg(*obGatewayPtr->obBatchMsg);
            obGatewayPtr->obBatchMsg=NULL;
        }
    }
}
