#pragma once

#include "TransList.h"
#include "MsgPool.h"

namespace JaCoAP
{
class TransMgr
{
	TransList _transList;

public:
	TransMgr();

	Transaction * GetTrans();
	void PutTrans(Transaction *trans);

	void CleanupTrans();
};
}

