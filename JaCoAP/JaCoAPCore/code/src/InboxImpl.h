#pragma once

#include <ErrorCodes.h>
#include <Inbox.h>

namespace JaCoAP
{
class CoapStack;
class TransProcessor;
}

namespace JaCoAP
{
class InboxImpl : public Inbox
{
private:
    TransProcessor    *_transProcessor = nullptr;
    CoapStack         *_coapStack      = nullptr;

public:
    virtual ~InboxImpl ();

    ErrCode receivePacket( Packet *pkt ) override;
    ErrCode receiveRequest( Request *req ) override;
    ErrCode receiveResponse( Response *res ) override;
    ErrCode receiveEmptyMsg( Msg *emptyMsg ) override;

    void setTransProcessor( TransProcessor *transProcessor ) { _transProcessor = transProcessor; }

    void setCoapStack( CoapStack *coapStack ) { _coapStack = coapStack; }

private:
    void reject( Msg *msg ) const;

    void reject( IpSocketAddr *dstSocketAddr, MsgHdr *msgHdr ) const;
};
}