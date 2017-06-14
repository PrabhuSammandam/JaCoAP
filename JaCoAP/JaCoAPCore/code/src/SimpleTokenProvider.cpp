#include "SimpleTokenProvider.h"

namespace JaCoAP {

SimpleTokenProvider::SimpleTokenProvider( ) {
}

u64 SimpleTokenProvider::AllocToken( Msg * msg ) {
	return u64( );
}

void SimpleTokenProvider::FreeToken( u64 token ) {
}

bool SimpleTokenProvider::IsTokenInUse( u64 token ) {
	return false;
}

}