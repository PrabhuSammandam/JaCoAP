#pragma once

#include "ja_types.h"
#include "Transaction.h"

namespace JaCoAP {

#define MAX_DEDUPLICATE_HASH_ENTRIES    20

struct DeduplicatorHashEntry {
	Transaction* trans;
	u32 hashKey;
	u8 used;
};

class DeduplicatorHashList {
	DeduplicatorHashEntry _list[MAX_DEDUPLICATE_HASH_ENTRIES];
	u8 _count = 0;
	u8 _size = MAX_DEDUPLICATE_HASH_ENTRIES;

public:
	DeduplicatorHashList();

	void Reset();

	Transaction* PutIfAbsent(u32 hashKey, Transaction* trans);
	u16 CreateEntry(u32 hashKey, Transaction* trans);
	Transaction* Find(u32 hashKey) const;
	Transaction* Remove(u32 hashKey);
	u8 Count() const { return _count; }
	bool IsEmpty() const { return _count == 0; }
};

class Deduplicator {
	DeduplicatorHashList _hashList;

public:
	Deduplicator();

	bool IsEmpty() const;
	Transaction* Find(u32 keyMid) const;
	Transaction* FindOrAdd(u32 keyMid, Transaction* trans);
};

}
