#include "CUnit.h"
#include "IpAddress.h"
#include "SimpleAllocator.h"
#include <vector>
#include <forward_list>
#include "Option.h"
#include "Msg.h"
#include <list>
#include "OptionsSet.h"
#include "Transaction.h"
#include <memory>
#include <functional>

using namespace JaCoAP;

u32 fnv_32a_buf(void* buf, size_t len, u32 hval) {
	auto bp = static_cast<unsigned char *>(buf); /* start of buffer */
	auto be = bp + len; /* beyond end of buffer */

	/*
	* FNV-1a hash each octet in the buffer
	*/
	while (bp < be) {

		/* xor the bottom with the current octet */
		hval ^= u32(*bp++);

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
		hval *= FNV_32_PRIME;
#else
		hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
	}

	/* return our new hash value */
	return hval;
}

static void TEST_Option_IpAddress() {
	IpAddress addr;
	addr.Print();

	u32 ipAddr = 0x10203040;
	u32 hash = 0;

	hash = fnv_32a_buf(pvoid(&ipAddr), sizeof(u32), hash);
	printf("Hash %u\n", hash);

	ipAddr = 0x10203041;
	hash = fnv_32a_buf(pvoid(&ipAddr), sizeof( u32), 0);
	printf("Hash %u\n", hash);

//	CU_ASSERT( (addr[0] == 0) && (addr[1] == 0) && (addr[2] == 0) && (addr[3] == 0) );

	u8 addr_byte[4] = {0x10,0x20,0x30,0x40};

	IpAddress addr1(0x10203040);
	addr1.Print();
//	CU_ASSERT_EQUAL( 0x10203040, u32( addr1 ) );

	IpAddress addr4(0x10, 0x20, 0x30, 0x40);
	addr4.Print();
//	CU_ASSERT_EQUAL( 0x10203040, u32( addr4 ) );

//	u32 a = u32(addr1);
//	CU_ASSERT_EQUAL( 0x10203040, a );
}

class test {
	int i = 0;
public:
	void* operator new(size_t size) {
		std::cout << "Allocating memory";
		return ::operator new(size);
	}

	void operator delete(void* ptr) {
		std::cout << "DeAllocating memory";
		::operator delete(ptr);
	}
};

template <typename T>
using unique_ptr_deleter = std::unique_ptr<T, void( *)(T*)>;

using BufferPtr = std::unique_ptr<uint8_t, void( *)(uint8_t*)>;
auto ByteArrayDeleter = [] ( uint8_t* buf ) {
	operator delete(buf);
};

class Buffer {
	unique_ptr_deleter<uint8_t> _buf{nullptr, ByteArrayDeleter};
public:
	Buffer() {
		auto b = new uint8_t[100];
		_buf = unique_ptr_deleter<uint8_t>(b, ByteArrayDeleter);
	}
};

class B {
public:
	std::unique_ptr<int> i;

	B() : i(new int(0)) {
	}

	void* operator new(size_t size) {
		std::cout << "Allocating memory";
		return ::operator new(size);
	}

	void operator delete(void* ptr) {
		std::cout << "DeAllocating memory";
		::operator delete(ptr);
	}
};

static void TEST_Option_CustomAllocator() {
	std::list<Transaction, JaCoAP::SimpleAllocator<Transaction>> v;

	Transaction trans;
	trans.Timestamp(100);
	v.push_back(std::move(trans));

	ByteArray buf(10);

	buf.GetArray( )[0] = 'a';

	ByteArray buf2 = std::move(buf);

	//	std::cout << v.

	//	v.reserve( 20 );
	//	v.push_back( Msg() );
	//	v.push_back( Msg( ) );
	//	v.push_back( Msg( ) );
	//	v.push_back( Msg( ) );
	//	v.push_back( Msg( ) );
	//	v.push_back( Msg( ) );
	//	v.push_back( Msg( ) );
	//	v.push_back( Msg( ) );

	//	for ( auto it = v.begin( ); it != v.end( ); ++it) {
	//		auto msg = *it;
	//	}
}

void TEST_Option_TemplateBitSet() {
	OptionsSet set;

	set.AddOption<u16>( 10, 10 );

	auto msgSize = sizeof( Msg );

	std::cout << msgSize;

	ByteArray byteArray{4};

	byteArray.SetArray( 0, 0 );
}

void TEST_SUITE_IpAddress() {
	auto group = CU_add_suite("IpAddress tests", nullptr, nullptr);

	//CU_add_test( group, "IpAddress", TEST_Option_IpAddress );
//	CU_add_test(group, "Custom Allocator", TEST_Option_CustomAllocator);
			CU_add_test( group, "TemplateBitSet", TEST_Option_TemplateBitSet );
}
