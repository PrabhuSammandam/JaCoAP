#include <stdio.h>
#include "MsgPool.h"

namespace JaCoAP {

MsgPool::MsgPool( ) {
	for ( auto i = 0; i < JA_COAP_CONFIG_MAX_MSGS; i++ ) {
		_msgList[i].used = false;
	}
}

Msg * MsgPool::GetMsg( ) {
	for ( auto i = 0; i < JA_COAP_CONFIG_MAX_MSGS; i++ ) {
		if ( _msgList[i].used == false ) {
			_msgList[i].used = true;
			_msgList[i].msg.Reset( );

			printf( "Alloced MSG idx[%d], addr[%p]\n", i, &_msgList[i].msg );

			return &_msgList[i].msg;
		}
	}

	return nullptr;
}

void MsgPool::PutMsg( Msg * msg ) {
	for ( auto i = 0; i < JA_COAP_CONFIG_MAX_MSGS; i++ ) {
		if ( &_msgList[i].msg == msg ) {
			printf( "Released MSG idx[%d], addr[%p]\n", i, &_msgList[i].msg );
			msg->Reset( );
			_msgList[i].used = false;
			break;
		}
	}
}

}