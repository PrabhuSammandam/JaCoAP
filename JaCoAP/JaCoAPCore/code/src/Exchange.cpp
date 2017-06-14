/*
 * Exchange.cpp
 *
 *  Created on: May 21, 2017
 *      Author: psammand
 */

#include <ByteArray.h>
#include <Exchange.h>
#include <MemMgr.h>
#include <string.h>
#include <algorithm>
#include "JaCoapIF.h"

FILE_NAME("Exchange.cpp");

namespace JaCoAP
{
Exchange::Exchange ()
{
}

Exchange::Exchange( Transaction *trans, Resource *resource ) :
    _trans{ trans }, _resource{ resource }
{
}

Exchange::Exchange( Exchange &&move ) noexcept
{
    *this = std::move( move );     // <- Important
}

Exchange & Exchange::operator = ( Exchange &&move ) noexcept
{
    // Avoid self assignment
    if( &move == this )
    {
        return ( *this );
    }

    this->_resource = move._resource;
    this->_trans    = move._trans;
    move._trans     = nullptr;
    move._trans     = nullptr;

    return ( *this );
}

Exchange::~Exchange ()
{
    this->_resource = nullptr;
    this->_trans    = nullptr;
}

void Exchange::Respond( MsgCode code )
{
    auto response = mnew Request();

    response->Code( code );

    Respond( response );
}

void Exchange::Respond( uint8_t *payload )
{
    Respond( MsgCode::CONTENT_205, payload );
}

void Exchange::Respond( ByteArray *payload )
{
    Respond( MsgCode::CONTENT_205, payload );
}

void Exchange::Respond( MsgCode code, ByteArray *payload, ContentFormatId contentFormat )
{
    auto response = mnew Request();

    response->Code( code );
    response->Payload( payload );

    if( contentFormat != ContentFormatId::NONE )
    {
        // response->GetOptionSet()->SetContentFormat(contentFormat);
    }

    Respond( response );
}

void Exchange::Respond( MsgCode code, uint8_t *payload, ContentFormatId contentFormat )
{
    uint16_t plLen       = uint16_t( strlen( (const char *) payload ) );
    auto     byteArrayPl = mnew ByteArray( payload, plLen );

    Respond( code, byteArrayPl, contentFormat );
}

void Exchange::Respond( Response *response )
{
    if( response == nullptr )
    {
        return;
    }

    _trans->SendResponse( response );
}
}
