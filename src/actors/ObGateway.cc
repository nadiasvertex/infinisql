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
 * @file   ObGateway.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 09:10:25 2014
 * 
 * @brief  Outbound Gateway actor. Counterpart to IbGateway. Receives messages
 * from actors on current node bound for remote nodes. Sends them over the
 * network to IbGateway.
 */

#include "ObGateway.h"

ObGateway::ObGateway(Actor::identity_s identity)
    : Actor(identity)
{
}

void ObGateway::operator()()
{

    while(1)
    {
        sleep(10);
    }
}
