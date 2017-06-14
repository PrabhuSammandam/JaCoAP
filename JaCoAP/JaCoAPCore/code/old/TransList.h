#ifndef __TRANS_LIST_H__
#define __TRANS_LIST_H__

#include "ja_types.h"
#include "JaCoapConfig.h"
#include "Transaction.h"
#include "IpSocketAddr.h"
#include <cstdint>

namespace JaCoAP {

struct TransListEntry {
	Transaction _trans;
	bool _used;
};
	
using ckForEachTrans = void( *)(Transaction* trans);

class TransList {
	TransListEntry _transEntryList[JA_COAP_CONFIG_MAX_TRANSACTIONS];
	u8 _count = 0;

public:
	TransList();
	~TransList();

	Transaction* AllocTrans();
	void FreeTrans(Transaction* trans);
	void LoopTrans(ckForEachTrans ckFunc);
	u8 Count( ) const;

	Transaction* FindTransaction(IpSocketAddr& remoteEp, u16 msgId) const;
	Transaction* FindOrCreateTrans(IpSocketAddr& remoteEp, u16 msgId);
private:
	Transaction* FindEmptyTrans();
	Transaction* FindOldTrans();
};

}

#endif /*__TRANS_LIST_H__*/
