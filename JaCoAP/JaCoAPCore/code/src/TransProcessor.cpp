#include <IpAddress.h>
#include <IpSocketAddr.h>
#include <ja_types.h>
#include <Msg.h>
#include <OptionsSet.h>
#include <stdio.h>
#include <Transaction.h>
#include <TransProcessor.h>
#include <Utils.h>
#include "JaCoapIF.h"

FILE_NAME( "TransProcessor.cpp" );

namespace JaCoAP
{
TransProcessor::TransProcessor () : _transStore()
{
}

void TransProcessor::sendRequest( Transaction *trans, Request *request )
{
    _transStore.RegisterRequest( trans );
    // printf("TransProcessor::SendRequest=> Tracking new req MsgId[%u], Tkn[%llu] \n", request->Id(), request->Tkn());
}

/* Sending empty message is to ACK or to stop getting the request/response.
 * Only ACK and RST message type is valid for Empty message */
void TransProcessor::sendEmptyMsg( Transaction *trans, Msg *msg ) const
{
    dbg_info( "msg type %u", (u8) msg->Type() );

    if( ( msg != nullptr ) && msg->IsRst() && ( trans != nullptr ) )
    {
        trans->Complete( true );
    }
}

void TransProcessor::sendResponse( Transaction *trans, Response *response )
{
    if( trans == nullptr )
    {
        dbg_error( "trans null, exiting" );
        return;
    }

    if( response == nullptr )
    {
        dbg_error( "response null, exiting" );
        return;
    }

    response->Tkn( trans->CurReq()->Tkn() ); // set the same token as received from request

    dbg_info( "type[%u]", (u8) response->Type() );

    // If this is a CON notification we now can forget all previous NON notifications
    if( response->IsConfirmable() || response->IsAck() )
    {
    }

    /* check for the block-wise transfer is ongoing */
    if( response->GetOptionSet()->HasBlock2() )
    {
        // Block-wise transfers are identified by URI and remote end-point
        auto keyUri = Utils::HashByUri( trans->CurReq(), *response->DstSocketAddr() );

        dbg_info( "has block2 with URI_KEY[0x%x]", keyUri );

        if( ( trans->GetResBlockTransferStatus() != nullptr ) && !response->GetOptionSet()->HasObserve() )
        {
            /* In block-wise transfer, first req is the original request and it is stored in trans->Req()
             * and all other subsequent requests are stored in trans->CurReq(). So keep the original request
             * and delete current request in the response*/
            if( trans->Req() != trans->CurReq() )
            {
                dbg_info( "Deleting current req" );
                mdelete trans->CurReq();
                trans->CurReq( nullptr );
            }

            // Register ongoing block-wise GET requests
            dbg_info( "Registering the ongoing block-wise transfer with URI_KEY[0x%x]", keyUri );
            _transStore.RegisterByURI( keyUri, trans );
        }
        else
        {
            // Observe notifications, only send the first block, hence do not store them as ongoing
            dbg_info( "Ongoing Block2 completed, unregistering trans with URI_KEY[0x%x]\n", keyUri );
            _transStore.RemoveByUriKey( keyUri );
        }
    }

    // Register CON msg to match for ACK or RST
    if( response->IsConfirmable() )
    {
        dbg_info( "registering response" );
        _transStore.RegisterResponse( trans );
    }
    else if( response->IsNonConfirmable() )
    {
        if( response->GetOptionSet()->HasObserve() )
        {
            /* Need to register it so that we can match an RST sent by a peer that wants to
             * cancel the observation. This NON msg response will later be removed from the
             * exchange store when ExchangeObserverImpl.completed() is called */
            _transStore.RegisterResponse( trans );
        }
        else
        {
            /* No need to register this response since it is NON msg and we are not
             * expecting the ACK or RST msg. Also set the new message Id for this NON message */
            _transStore.AssignMsgId( response );
        }
    }

    // Only CONs and Observe keep the exchange active (CoAP server side)
    if( !response->IsConfirmable() && response->IsLastRes() )
    {
        dbg_info( "completing trans" );
        trans->Complete( true );
    }
}

/*
 * This request could be
 *  - Complete origin request => deliver with new exchange
 *  - One origin block        => deliver with ongoing exchange
 *  - Complete duplicate request or one duplicate block (because client got no ACK)
 *      =>
 *               if ACK got lost              => re-send ACK
 *               if ACK+response got lost     => res-end ACK+response
 *               if nothing has been sent yet => do nothing
 * (Retransmission is supposed to be done by the retransmission layer)
 */
Transaction * TransProcessor::receiveRequest( Request *request )
{
    if( request == nullptr )
    {
        dbg_error( "Passed NULL request" );
        return ( nullptr );
    }

    auto keyMid = request->GetHashBySrcEpAndMid();

    dbg_info( "Received request with MID_KEY[0x%x]", keyMid );

    /* Check the request needs block-wise transfer. If it is not block-wise transfer
     * then no need to have the uri key and msgid key
     */
    if( !request->HasBlockOption() )
    {
        dbg_info( "request has no block option" );
        auto newTrans = mnew Transaction( TransOrigin::REMOTE, request );

        auto prevTrans = _transStore.FindOrAdd( keyMid, newTrans );

        if( prevTrans != nullptr )
        {
            mdelete newTrans;
            dbg_info( "Duplicate request" );
            request->SetDuplicate( true );

            return ( prevTrans );
        }

        return ( newTrans );
    }

    auto keyUri       = Utils::HashByUri( request, *request->SrcSocketAddr() );
    auto ongoingTrans = _transStore.GetByUriKey( keyUri );

    dbg_info( "request has block option" );

    // printf("TransProcessor::ReceiveRequest=> Checking for ongoing response with KeyUri[%u], ip[%x], port[%x]\n", keyUri,
    // request->SrcSocketAddr()->IpAddr()->AsU32(), request->SrcSocketAddr()->Port());

    if( ongoingTrans != nullptr )
    {
        dbg_info( "There is ongoing transaction for URI_KEY[0x%x]", keyUri );
        auto prevTrans = _transStore.FindOrAdd( keyMid, ongoingTrans );

        if( prevTrans != nullptr )
        {
            dbg_info( "duplicate req for ongoing trans" );
            request->SetDuplicate( true );
        }
        else
        {
            // the exchange is continuing, we can (i.e., must) clean up the previous response
            // check for null, in case no response was created (e.g., because the resource handler crashed...)
            auto ongoingResponse = ongoingTrans->CurRes();

            if( ( ongoingResponse != nullptr )
              && !ongoingResponse->IsAck()
              && !ongoingResponse->GetOptionSet()->HasObserve() )
            {
                keyMid = ongoingResponse->GetHashByDstEpAndMid();

                dbg_info( "Ongoing trans got new req, deleting old entry MID_KEY[0x%x]", keyMid );
                _transStore.RemoveByMidKey( keyMid );
            }
        }

        return ( ongoingTrans );
    }

    dbg_info( "no ongoing trans, so creating new trans" );

    // We have no ongoing exchange for that request block.

    /*
     * Note the difficulty of the following code: The first message of a blockwise transfer might arrive
     * twice due to a retransmission. The new Exchange must be inserted in both the hash map 'ongoing'
     * and the deduplicator. They must agree on which exchange they store!
     */
    auto newTrans = mnew Transaction( TransOrigin::REMOTE, request );
    auto prevTrans = _transStore.FindOrAdd( keyMid, newTrans );

    if( prevTrans != nullptr )
    {
        mdelete newTrans;
        request->SetDuplicate( true );

        return ( prevTrans );
    }

    dbg_info( "registering new trans with URI_KEY[0x%x]", keyUri );
    _transStore.RegisterByURI( keyUri, newTrans );

    return ( newTrans );
}

/*
 * This response could be
 * - The first CON/NCON/ACK+response => deliver
 * - Retransmitted CON (because client got no ACK)
 *               => resend ACK
 */
Transaction * TransProcessor::receiveResponse( Response *response )
{
    u32  keyMid = response->GetHashBySrcEpAndMid();
    u32  keyTkn = response->GetHashBySrcEpAndTkn();

    auto trans = _transStore.GetByTokenKey( keyTkn );

    if( trans == nullptr )
    {
        /* Client doesn't started any request for this response. */
        if( !response->IsAck() )
        {
            /* the response type may be CON or NON or RST. So check in deduplicator whether we
             * already got the same response */
            auto prevTrans = _transStore.Find( keyMid );

            if( prevTrans != nullptr )
            {
                /* already there is an transaction with this same response so mark it as duplicate */
                response->SetDuplicate( true );

                return ( prevTrans );
            }
        }
        else
        {
            /* this is ACK response i.e piggy-backed response, so discard it */
            dbg_error( "discarding unmatchable piggy-backed response" );
        }

        return ( nullptr );
    }
    else
    {
        if( response->IsAck() && ( trans->CurReq()->Id() != response->Id() ) )
        {
            dbg_error( "response not matched with requested message id" );
            // The token matches but not the MID.
            return ( nullptr );
        }

        // we have received a Response matching the token of an ongoing Exchange's Request
        // according to the CoAP spec (https://tools.ietf.org/html/rfc7252#section-4.5),
        // message deduplication is relevant for CON and NON messages only
        if( ( response->IsConfirmable() || ( response->IsNonConfirmable() ) ) && ( _transStore.FindOrAdd( keyMid, trans ) != nullptr ) )
        {
            response->SetDuplicate( true );
        }
        else
        {
            // we have received the expected response for the original request
            keyMid = trans->CurReq()->GetHashByDstEpAndMid();

            if( _transStore.RemoveByMidKey( keyMid ) != nullptr )
            {
                dbg_info( "closed open request" );
            }
        }
    }

    return ( trans );
}

Transaction * TransProcessor::receiveEmptyMsg( Msg *emptyMsg )
{
    /* an empty ACK or RST is always received as a reply to a message
     * exchange originating locally, i.e. the message will have an MID
     * that has been created here
     */
    auto keyMid = emptyMsg->GetHashBySrcEpAndMid();

    /* No need of this transaction since we got the ACK or RST and remove from the transaction store. */
    auto oldTrans = _transStore.RemoveByMidKey( keyMid );

    if( oldTrans != nullptr )
    {
        dbg_info( "received expected msg with MID_KEY[0x%x] and unregistering the trans", keyMid );
    }
    else
    {
        dbg_info( "ignoring the unexpected msg with MID_KEY[0x%x]", keyMid );
    }

    return ( oldTrans );
}
}
