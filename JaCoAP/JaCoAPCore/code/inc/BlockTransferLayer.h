#pragma once

#include <BaseIStackLayer.h>
#include <ErrorCodes.h>
#include <ja_types.h>
#include <Msg.h>

namespace JaCoAP
{
class Block2Info;
class BlockOption;
}

namespace JaCoAP
{
class BlockTransferLayer : public BaseStackLayer
{
    u16    _maxPayloadSize = 128;
    u16    _maxResBodySize = 128;
    u16    _prefBlockSize  = 128;

public:

    BlockTransferLayer ();

    ErrCode sendRequest( Transaction *trans, Request *request ) override;
    ErrCode sendResponse( Transaction *trans, Response *response ) override;
    ErrCode receiveRequest( Transaction *trans, Request *request ) override;
    ErrCode receiveResponse( Transaction *trans, Response *response ) override;

private:
    ErrCode   handleInboundBlockUpload( Transaction *trans, Msg *request );
    void      SendNextBlock( Transaction *trans, Msg *msg, BlockOption *block1, Block2Info *block2Info );
    ErrCode   handleBlock1Res( Transaction *trans, Msg *msg );
    ErrCode   handleBlock2Res( Transaction *trans, Msg *msg );
    Block2Info* getResBlock2Info( Transaction *trans, Msg *response ) const;
    ErrCode   sendErrorRes( Transaction *trans, Msg *request, BlockOption block1, pu8 payloadStr );
    Block2Info* getReqBlock2Info( Transaction *trans, Msg *request ) const;
    Msg       * getNextResBlock( Msg *parentResponse, Block2Info *blkTfrStatus );
    Msg       * getNextReqBlock( Msg *request, Block2Info *block2Info );
    bool      isBlockTransferRequired( Msg *request ) const;
    bool      isBlockTransferRequired( Transaction *trans, Msg *response ) const;
    void      startEarlyBlock2Negotiation( Transaction *trans, Msg *request ) const;
};
}