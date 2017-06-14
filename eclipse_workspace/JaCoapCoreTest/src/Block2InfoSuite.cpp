#include <Block2Info.h>
#include <cute_equals.h>
#include <cute_test.h>
#include <ja_types.h>
#include <Option.h>
#include "Block2InfoSuite.h"
#include <vector>

using namespace JaCoAP;

void Block2InfoConstructorTest() {
	Block2Info info{};
}

void Block2InfoMemberFunctionTest(){
	Block2Info info{};

	ASSERT_EQUAL(false, info.IsComplete());
	ASSERT_EQUAL(false, info.IsRandomAccess());
	ASSERT_EQUAL(0, info.GetDataBufSize());
	ASSERT_EQUAL(0, info.GetSZX());
	ASSERT_EQUAL(0, info.GetCurNum());
	ASSERT_EQUAL(-1, info.GetObsSeqNum());
	ASSERT_EQUAL(u16(ContentFormatId::TEXT__PLAIN), info.GetContentFormat());
	ASSERT_EQUAL(nullptr, info.GetData());

	info.SetComplete(true);
}

cute::suite make_suite_Block2InfoSuite(){
	cute::suite s;
	s.push_back(CUTE(Block2InfoConstructorTest));
	s.push_back(CUTE(Block2InfoMemberFunctionTest));
	return s;
}



