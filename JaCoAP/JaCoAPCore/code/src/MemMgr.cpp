/*
 * MemMgr.cpp
 *
 *  Created on: May 19, 2017
 *      Author: psammand
 */

#include <stdio.h>
#include <Msg.h>
#include <Transaction.h>
#include "JaCoapConfig.h"
#include "MemMgr.h"
#include <cstdint>
#include "JaCoapIF.h"

FILE_NAME( "MemMgr.cpp" );

namespace JaCoAP
{
struct MsgPoolEntry
{
    Msg        msg;
    uint8_t    used;
    uint8_t    count;
    MsgPoolEntry () :
        msg{}, used{ false }, count{ 0 }
    {
    }
};

struct TransListEntry
{
    Transaction    _trans;
    bool           _used;
    uint8_t        count;
    TransListEntry () :
        _trans{}, _used{ false }, count{ 0 }
    {
    }
};

static MsgPoolEntry   _msgPoolList[JA_COAP_CONFIG_MAX_MSGS] {};
static TransListEntry _transPoolList[JA_COAP_CONFIG_MAX_TRANSACTIONS] {};

Msg * JaCoAP::MemMgr::AllocMsg( void )
{
    for( auto i = 0; i < JA_COAP_CONFIG_MAX_MSGS; i++ )
    {
        if( _msgPoolList[i].used == false )
        {
            _msgPoolList[i].used = true;
            _msgPoolList[i].count++;

//            dbg_info( "Allocated MSG [%p]", &_msgPoolList[i].msg );

            return ( &_msgPoolList[i].msg );
        }
    }

    return ( nullptr );
}

void JaCoAP::MemMgr::FreeMsg( Msg *msg )
{
    for( auto i = 0; i < JA_COAP_CONFIG_MAX_MSGS; i++ )
    {
        if( &_msgPoolList[i].msg == msg )
        {
//            dbg_info( "Released MSG [%p]", &_msgPoolList[i].msg );

            _msgPoolList[i].used = false;
            _msgPoolList[i].count--;
            break;
        }
    }
}

Transaction * JaCoAP::MemMgr::AllocTrans( void )
{
    for( auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++ )
    {
        if( _transPoolList[i]._used == false )
        {
            _transPoolList[i]._used = true;
            _transPoolList[i].count++;

//            dbg_info( "Allocated trans [%p]", &_transPoolList[i]._trans );
            return ( &_transPoolList[i]._trans );
        }
    }

    return ( nullptr );
}

void JaCoAP::MemMgr::FreeTrans( Transaction *trans )
{
    for( auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++ )
    {
        if( &_transPoolList[i]._trans == trans )
        {
            _transPoolList[i]._used = false;
            _transPoolList[i].count--;
//            dbg_info( "Released trans [%p]\n", &_transPoolList[i]._trans );
            break;
        }
    }
}

}
