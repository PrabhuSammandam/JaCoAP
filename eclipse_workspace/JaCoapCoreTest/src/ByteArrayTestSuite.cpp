#include <ByteArray.h>
#include <cute_test.h>
#include <string.h>
#include "ByteArrayTestSuite.h"
#include <algorithm>
#include <cstdint>
#include <vector>

using namespace JaCoAP;

void ByteArrayTestEmptyConstructor()
{
	ByteArray array {};

}

void ByteArrayTestSizeConstructor()
{
	ByteArray array2 { 100 };
}

void ByteArrayTestOtherByteArrayConstructor()
{
	auto constPtr = "This is const string";

	ByteArray constArray { (uint8_t*) constPtr,
	                       uint16_t(strlen(constPtr)) };
}

void ByteArrayTestMoveConstructor()
{
	auto constPtr = "This";

	ByteArray constArray { (uint8_t*) constPtr,
	                       uint16_t(strlen(constPtr)) };

	ByteArray moveArray = std::move(constArray);
}

void ByteArrayTestCopyStr()
{
	ByteArray byteArray { 10 };

	byteArray.CopyStr(nullptr);
	byteArray.CopyStr((uint8_t*) "");
	byteArray.CopyStr((uint8_t*) "test");
	byteArray.CopyStr((uint8_t*) "test this");
	byteArray.CopyStr((uint8_t*) "copy again");
}

void ByteArrayTestCopyNBytes()
{
	ByteArray byteArray { 4 };

	byteArray.CopyNBytes(nullptr, 0);
	byteArray.CopyNBytes((uint8_t*) "test this", 7);
}

void ByteArrayTestCopyTo()
{
	auto constPtr = "1234567890";

	ByteArray constArray { (uint8_t*) constPtr,
	                       uint16_t(strlen(constPtr)) };
	ByteArray byteArray { 4 };
	ByteArray byteArray1 { 2 };

	constArray.CopyTo(nullptr, 0, 2);
	constArray.CopyTo(nullptr, 0, 0);
	constArray.CopyTo(&byteArray, 0, 0);
	constArray.CopyTo(&byteArray, 100, 0);
	constArray.CopyTo(&byteArray, 0, 2);
	constArray.CopyTo(&byteArray, 10, 2);
	constArray.CopyTo(&byteArray1, 0, 4);

	byteArray1.SetArray(0, 0);
}

cute::suite make_suite_ByteArrayTestSuite()
{
	cute::suite s {};
	//s.push_back(CUTE(ByteArrayTestEmptyConstructor));
	//s.push_back(CUTE(ByteArrayTestSizeConstructor));
//	s.push_back(CUTE(ByteArrayTestOtherByteArrayConstructor));
//	s.push_back(CUTE(ByteArrayTestMoveConstructor));
//	s.push_back(CUTE(ByteArrayTestCopyStr));
//	s.push_back(CUTE(ByteArrayTestCopyNBytes));
	s.push_back(CUTE(ByteArrayTestCopyTo));
	return s;
}

