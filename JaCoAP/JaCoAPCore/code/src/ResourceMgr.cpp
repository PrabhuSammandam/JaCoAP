#include <Msg.h>
#include <Option.h>
#include <OptionsSet.h>
#include <ResourceMgr.h>
#include <string.h>
#include <Transaction.h>
#include <JaCoapConfig.h>
#include <Resource.h>
#include <WellKnownRes.h>

namespace JaCoAP
{
ResourceMgr ResourceMgr::_resMgr;

uint8_t     _uriPath[JA_COAP_CONFIG_URI_PATH_MAX_SIZE];

ByteArray _uriByteArray{ &_uriPath[0],
                         JA_COAP_CONFIG_URI_PATH_MAX_SIZE,
                         false };
WellKnownRes _wkr;
ResourceMgr::ResourceMgr ()
{
    for( auto i = 0; i < MAX_RESOURCES; i++ )
    {
        _resourceList[i] = nullptr;
    }

    _resourceCount = 0;

    AddResource( &_wkr );
}

ErrCode ResourceMgr::AddResource( Resource *resource )
{
    if( _resourceCount >= MAX_RESOURCES )
    {
        return ( ErrCode::OUT_OF_MEMORY );
    }

    for( auto i = 0; i < MAX_RESOURCES; i++ )
    {
        if( _resourceList[i] == nullptr )
        {
            _resourceList[i] = resource;
            _resourceCount++;
            break;
        }
    }

    return ( ErrCode::OK );
}

Resource * ResourceMgr::FindResource( Request *req )
{
    req->GetOptionSet()->GetUriPathString( _uriByteArray );
    auto hashValue = _uriByteArray.GetHashValue();

    for( auto i = 0; i < MAX_RESOURCES; i++ )
    {
        if( _resourceList[i] == nullptr )
        {
            continue;
        }

        if( hashValue == _resourceList[i]->getHashValue() )
        {
            return ( _resourceList[i] );
        }
    }

    return ( nullptr );
}

uint16_t ResourceMgr::GetResourceDiscoveryPayloadSize()
{
    return ( 0 );
}

void ResourceMgr::GetResourceDiscoveryPayload( ByteArray *resourceDiscoverData )
{
}
}