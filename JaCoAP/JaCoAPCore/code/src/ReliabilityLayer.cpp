// File name            : ReliabilityLayer.cpp
//
// ___ _       _        ___            _
// |_  (_)     (_)      / _ \          | |
// | |_ _ __  _ _   _/ /_\ \_ __ ___ | | __ _
// | | | '_ \| | | | |  _  | '_ |
///\__/ / | | | | | |_| | | | | | | | | | | (_| |
// \____/|_|_| |_| |\__,_\_| |_/_| |_| |_|_|\__,_|
// _/ |
// |__/
//

#include <Msg.h>
#include <ReliabilityLayer.h>
#include <Transaction.h>
#include <Utils.h>
#include <cstdio>
#include "JaCoapIF.h"

FILE_NAME( "ReliabilityLayer.cpp" );

namespace JaCoAP
{
ErrCode ReliabilityLayer::sendRequest( Transaction *trans, Msg *request )
{
    if( request->Type() == MsgType::NONE )
    {
        /* if the request message type is not set, then set the message type as CON */
        request->Type( MsgType::CON );
    }

    if( request->IsConfirmable() )
    {
        // configure the re-transmit details
    }

    return ( BaseStackLayer::sendRequest( trans, request ) );
}

/* COMPLETE */

/*
 * Makes sure that the response type is correct.
 * 1. The response type for a NON request can be NON or CON.
 * 2. The response type for a CON request should either be an ACK with a piggy-backed response or,
 *    if an empty ACK has already be sent, a CON or NON with a separate response should be send.
 */
ErrCode ReliabilityLayer::sendResponse( Transaction *trans, Response *response )
{
    auto curReq     = trans->CurReq();

    switch( response->Type() )
    {
        case MsgType::NONE:
        {
            /* Top layers not set the message type, so need to set the message type based on the request type.
             * The request type may be two types and based on that set response type
             * 1. CON message
             *              a. Already separate ACK sent, then set the current response as CON
             *              b. Else it is piggy-backed response, then set the current response as ACK
             * 2. NON message
             *              a. For NON msg, set the current response as NON itself.
             */
            if( curReq->IsConfirmable() )
            {
                if( curReq->IsAcknowledged() )
                {
                    /* Already acknowledged i.e separate ACK and CON.
                     * No need to set the Message Id, it will be set in the transaction processor
                     * And also no need to set acknowledged since it is already acknowledged*/
                    response->Type( MsgType::CON );
                }
                else
                {
                    /* this is piggy-backed response, so set acknowledged and also set the Message Id
                     * same as request */
                    curReq->SetAcknowledged( true );
                    response->Type( MsgType::ACK );
                    response->Id( curReq->Id() );
                }
            }
            else
            {
                /* request for this response is NON and so no need to set acknowledged
                 * and also no need to set the Message Id, it will be set in the transaction processor.
                 * For NON request, send the NON response*/
                response->Type( MsgType::NON );
            }
        }
        break;
        case MsgType::CON:
        {
            // configure the re-transmit details
        }
        break;
        case MsgType::NON:
        {
            /* nothing to do for NON response. For NON response new Message Id will be created
             * transaction processor*/
        }
        break;
        case MsgType::ACK:
        case MsgType::RST:
        {
            /* For ACK or RST, set the Message Id same as request */
            response->Id( curReq->Id() );
        }
        break;
        default:
        {
        }
        break;
    }

    return ( BaseStackLayer::sendResponse( trans, response ) );
}

/* COMPLETE */

/*
 * When we receive a duplicate of a request, we stop it here and do not forward it to the upper layer.
 * If the server has already sent a response, we send it again. If the request has only been
 * acknowledged (but the ACK has gone lost or not reached the client yet), we resent the ACK. If the
 * request has neither been responded, acknowledged or rejected yet, the server has not yet decided
 * what to do with the request and we cannot do anything.
 */
ErrCode ReliabilityLayer::receiveRequest( Transaction *trans, Msg *request )
{
    if( request->IsDuplicate() )
    {
        /* Request is a duplicate, so re-send ACK, RST or response */
        if( trans->CurRes() != nullptr )
        {
            dbg_info( "duplicate request received, sending response again" );
            /* Respond with the current response to the duplicate request */
            BaseStackLayer::sendResponse( trans, trans->CurRes() );
        }
        else if( trans->CurReq()->IsAcknowledged() )
        {
            dbg_info( "duplicate request received, sending ACK again" );
            /* this is not piggy-backed response case and already this request was acknowledged
             *  but no response computed yet. Retransmit ACK */
            auto    emptyAckMsg = Utils::getEmptyMsg( MsgType::ACK, request );

            sendEmptyMsg( trans, emptyAckMsg );
            mdelete emptyAckMsg;
        }
        else if( trans->CurReq()->IsRejected() )
        {
            dbg_info( "duplicate request received, sending RST again" );
            /* Already this request was rejected. Reject again */
            auto    emptyRstMsg = Utils::getEmptyMsg( MsgType::RST, request );

            sendEmptyMsg( trans, emptyRstMsg );
            mdelete emptyRstMsg;
        }
        else
        {
            // The server has not yet decided, whether to acknowledge or
            // reject the request. We know for sure that the server has
            // received the request though and can drop this duplicate here.
        }

        return ( ErrCode::OK );
    }

    trans->CurReq( request );

    return ( BaseStackLayer::receiveRequest( trans, request ) );
}

/* COMPLETE */

/*
 * When we receive a CON response, we acknowledge it and it also counts as acknowledgment
 * for the request. If the response is a duplicate, we stop it here and do not forward it to the upper layer.
 */
ErrCode ReliabilityLayer::receiveResponse( Transaction *trans, Msg *response )
{
    trans->RetransCnt( 0 );
    trans->CurReq()->SetAcknowledged( true );

    if( response->IsConfirmable() && !trans->Req()->IsCancelled() )
    {
        dbg_info( "sending ACK for received response" );
        /* acknowledging CON response */
        auto    emptyAckMsg = Utils::getEmptyMsg( MsgType::ACK, response );

        sendEmptyMsg( trans, emptyAckMsg );
        mdelete emptyAckMsg;
    }

    /* if the response is duplicate then no need to process further */
    if( response->IsDuplicate() )
    {
        dbg_info( "received duplicate response" );
        return ( ErrCode::OK );
    }

    return ( BaseStackLayer::receiveResponse( trans, response ) );
}

/* COMPLETE */

/*
 *  If we receive an ACK or RST, we mark the outgoing request or response as acknowledged or
 * rejected respectively and cancel its retransmission.
 */
ErrCode ReliabilityLayer::receiveEmptyMsg( Transaction *trans, Msg *emptyMsg )
{
    trans->RetransCnt( 0 );
    auto curMsg = ( trans->IsOriginLocal() ) ? trans->CurReq() : trans->CurRes();

    if( curMsg != nullptr )
    {
        if( emptyMsg->IsAck() )
        {
            curMsg->SetAcknowledged( true );
        }
        else if( emptyMsg->IsRst() )
        {
            curMsg->SetRejected( true );
        }
        else
        {
            dbg_error( "Received Empty Msg that is neither ACK or RST" );
        }
    }

    return ( BaseStackLayer::receiveEmptyMsg( trans, emptyMsg ) );
}
}
