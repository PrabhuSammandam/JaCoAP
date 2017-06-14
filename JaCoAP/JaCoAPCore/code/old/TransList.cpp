#include <stdio.h>
#include"TransList.h"

namespace JaCoAP {

TransList::TransList() {
	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		_transEntryList[i]._used = false;
	}
}

TransList::~TransList() {
	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		_transEntryList[i]._used = false;
	}
}

Transaction* TransList::AllocTrans() {
	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		if (_transEntryList[i]._used == false) {
			_transEntryList[i]._used = true;
			_count++;

			printf( "Alloc trans idx[%d], trans [%p]\n", i, &_transEntryList[i]._trans );
			return &_transEntryList[i]._trans;
		}
	}
	return nullptr;
}

void TransList::FreeTrans(Transaction* trans) {
	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		if (&_transEntryList[i]._trans == trans) {
			_transEntryList[i]._used = false;
			_count--;
			printf( "Release trans idx[%d], trans [%p]\n", i, &_transEntryList[i]._trans );
			break;
		}
	}
}

void TransList::LoopTrans(ckForEachTrans ckFunc) {
	for ( auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++ ) {
		if ( _transEntryList[i]._used == true ) {
			ckFunc( &_transEntryList[i]._trans );
		}
	}
}

u8 TransList::Count() const {
	return _count;
}

Transaction* TransList::FindOrCreateTrans(IpSocketAddr& remoteEp, u16 msgId) {
	Transaction* transaction = FindTransaction(remoteEp, msgId);

	if (transaction == nullptr) {
		transaction = FindEmptyTrans();

		if (transaction == nullptr) {
			transaction = FindOldTrans();
		}

		transaction->Reset();
		//transaction->RemoteEp( remoteEp );
	}
	return transaction;
}

Transaction* TransList::FindTransaction(IpSocketAddr& remoteEp, u16 msgId) const {
	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		if (!_transEntryList[i]._used) {
			continue;
		}

		//if ( (_transEntryList[i]._trans == remoteEp) && (_transEntryList[i]._trans == msgId) ) {
		//	return &_transEntryList[i]._trans;
		//}
	}
	return nullptr;
}

Transaction* TransList::FindEmptyTrans() {
	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		if (_transEntryList[i]._used == false) {
			return &_transEntryList[i]._trans;
		}
	}
	return nullptr;
}

Transaction* TransList::FindOldTrans() {
	u32 minTime = 0;
	Transaction* oldest = nullptr;

	for (auto i = 0; i < JA_COAP_CONFIG_MAX_TRANSACTIONS; i++) {
		if (!_transEntryList[i]._used) {
			continue;
		}

		if ((minTime == 0) || (_transEntryList[i]._trans.Timestamp() < minTime)) {
			oldest = &_transEntryList[i]._trans;
			minTime = _transEntryList[i]._trans.Timestamp();
		}
	}
	return (oldest != nullptr) ? oldest : &_transEntryList[0]._trans;
}

}
