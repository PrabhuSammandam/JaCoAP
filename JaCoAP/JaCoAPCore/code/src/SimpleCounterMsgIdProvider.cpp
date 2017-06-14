#include "SimpleCounterMsgIdProvider.h"

namespace JaCoAP {

SimpleCounterMsgIdProvider::SimpleCounterMsgIdProvider( ) {
	_msgCounter = 0x0005;
}

u16 SimpleCounterMsgIdProvider::GetNextMsgId( ) {
	return _msgCounter++;
}

}