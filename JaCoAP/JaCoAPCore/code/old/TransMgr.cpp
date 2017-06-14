#include "TransMgr.h"
#include "CoapMgr.h"

namespace JaCoAP
{

void FreeTrans(Transaction* trans);

TransMgr::TransMgr() :
		_transList {}
{
}

Transaction* TransMgr::GetTrans()
{
	return _transList.AllocTrans();
}

void TransMgr::PutTrans(Transaction*trans)
{
	_transList.FreeTrans(trans);
}

void TransMgr::CleanupTrans()
{
	if (_transList.Count() > 0)
	{
		_transList.LoopTrans(&FreeTrans);
	}
}

void FreeTrans(Transaction* trans)
{
	if (trans->IsComplete())
	{
		delete trans;
	}
}
}
