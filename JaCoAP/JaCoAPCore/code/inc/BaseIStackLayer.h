#pragma once

#include <ErrorCodes.h>
#include <IStackLayer.h>

namespace JaCoAP
{

class BaseStackLayer: public IStackLayer
{
	IStackLayer* _botLayer = nullptr;
	IStackLayer* _topLayer = nullptr;

public:
	ErrCode sendRequest(Transaction* trans, Msg* request) override
	{
		return (_botLayer != nullptr) ? _botLayer->sendRequest(trans, request) : ErrCode::NULL_POINTER;
	}

	ErrCode sendResponse(Transaction* trans, Msg* response) override
	{
		return (_botLayer != nullptr) ? _botLayer->sendResponse(trans, response) : ErrCode::NULL_POINTER;
	}

	ErrCode sendEmptyMsg(Transaction* trans, Msg* emptyMsg) override
	{
		return (_botLayer != nullptr) ? _botLayer->sendEmptyMsg(trans, emptyMsg) : ErrCode::NULL_POINTER;
	}

	ErrCode receiveRequest(Transaction* trans, Msg* request) override
	{
		return (_topLayer != nullptr) ? _topLayer->receiveRequest(trans, request) : ErrCode::NULL_POINTER;
	}

	ErrCode receiveResponse(Transaction* trans, Msg* response) override
	{
		return (_topLayer != nullptr) ? _topLayer->receiveResponse(trans, response) : ErrCode::NULL_POINTER;
	}

	ErrCode receiveEmptyMsg(Transaction* trans, Msg* emptyMsg) override
	{
		return (_topLayer != nullptr) ? _topLayer->receiveEmptyMsg(trans, emptyMsg) : ErrCode::NULL_POINTER;
	}

	void SetBottomLayer(IStackLayer* botLayer) override
	{
		_botLayer = botLayer;
	}

	void SetTopLayer(IStackLayer* topLayer) override
	{
		_topLayer = topLayer;
	}

	IStackLayer* GetBottomLayer() const
	{
		return _botLayer;
	}

	IStackLayer* GetTopLayer() const
	{
		return _topLayer;
	}

};

}

