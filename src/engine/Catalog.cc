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
 * @file   Catalog.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 08:14:02 2014
 * 
 * @brief  catalog is a collection of schemata and users
 * 
 * 
 */

#include "Catalog.h"

Catalog::Catalog() : Metadata (), nextuserid (0), nextschemaid (0),
                     nexttableid (0), nextindexid (0)
{
}

Catalog::Catalog(int16_t id, const std::string& name)
    : Catalog()
{
}

Catalog::Catalog(const Catalog &orig) : Metadata (orig)
{
    cp(orig);
}

Catalog &Catalog::operator= (const Catalog &orig)
{
    (Metadata)*this=Metadata(orig);
    cp(orig);
    return *this;
}

void Catalog::cp(const Catalog &orig)
{
    nextuserid=orig.nextuserid;
    nextschemaid=orig.nextschemaid;
    nexttableid=orig.nexttableid;
    nextindexid=orig.nextindexid;
}

Catalog::~Catalog()
{
}

void Catalog::ser(Serdes &output)
{
    Metadata::ser(output);
    output.ser(nextuserid);
    output.ser(nextschemaid);
    output.ser(nexttableid);
    output.ser(nextindexid);
}

size_t Catalog::sersize()
{
    size_t retval=Metadata::sersize();
    retval+=Serdes::sersize(nextuserid);
    retval+=Serdes::sersize(nextschemaid);
    retval+=Serdes::sersize(nexttableid);
    retval+=Serdes::sersize(nextindexid);

    return retval;
}

void Catalog::des(Serdes &input)
{
    Metadata::des(input);
    input.des(nextuserid);
    input.des(nextschemaid);
    input.des(nexttableid);
    input.des(nextindexid);
}

int16_t Catalog::getnextuserid()
{
    return ++nextuserid;
}

int16_t Catalog::getnextschemaid()
{
    return ++nextschemaid;
}

int16_t Catalog::getnexttableid()
{
    return ++nexttableid;
}

int16_t Catalog::getnextindexid()
{
    return ++nextindexid;
}

int Catalog::openEnvironment(std::string path)
{
    int retval=mdb_env_create(&lmdbinfo.env);
    if (retval)
    {
        return retval;
    }
    /* 96 bytes overhed per LMDB database per thread, according to
     * https://github.com/skydb/sky/pull/103
     * each table and index are a database
     */
    retval=mdb_env_set_maxdbs(lmdbinfo.env, 16384);
    if (retval)
    {
        mdb_env_close(lmdbinfo.env);
        lmdbinfo.env=nullptr;
        return retval;
    }
    retval=mdb_env_open(lmdbinfo.env, path.c_str(), MDB_WRITEMAP,
                        S_IRUSR|S_IWUSR);
    if (retval)
    {
        mdb_env_close(lmdbinfo.env);
        lmdbinfo.env=nullptr;
    }
    
    return retval;
}

void Catalog::closeEnvironment()
{
    mdb_env_close(lmdbinfo.env);
    lmdbinfo.env=nullptr;
}

int Catalog::deleteEnvironment(std::string path)
{
    string p=path;
    if (p.size() && p[p.size()-1] != '/')
    {
        p.append(1, '/');
    }
    string data=p + "data.mdb";
    int retval=remove(data.c_str());
    if (retval)
    {
        return retval;
    }
    string lock=p + "lock.mdb";
    return remove(lock.c_str());
}
