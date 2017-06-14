#pragma once

#include <ByteArray.h>
#include <cstdint>

namespace JaCoAP
{
class Exchange;
class ByteArray;
class Transaction;
}

namespace JaCoAP
{
class Resource
{
	uint32_t _hashValue = 0;
	ByteArray* _name = nullptr;
	ByteArray* _path = nullptr;
	bool _visible = true;
	bool _observable = false;

public:
	Resource(uint8_t* name, bool isVisible = true);
	Resource(ByteArray* name, bool isVisible = true);

	virtual ~Resource();

	void HandleRequest(Transaction* trans);

	virtual void HandleGet(Exchange* exchange);
	virtual void HandlePost(Exchange* exchange);
	virtual void HandlePut(Exchange* exchange);
	virtual void HandleDelete(Exchange* exchange);

	uint32_t getHashValue() const
	{
		return _hashValue;
	}

	void setHashValue(uint32_t hashValue = 0)
	{
		_hashValue = hashValue;
	}

	const ByteArray* getName() const
	{
		return _name;
	}

	void setName( ByteArray* name)
	{
		*_name = std::move(*name);
	}

	bool isObservable() const
	{
		return _observable;
	}

	void setObservable(bool observable = false)
	{
		_observable = observable;
	}

	const ByteArray* getPath() const
	{
		return _path;
	}

	void setPath( ByteArray* path)
	{
		*_path = std::move(*path);
	}

	bool isVisible() const
	{
		return _visible;
	}

	void setVisible(bool visible = true)
	{
		_visible = visible;
	}
};
}
