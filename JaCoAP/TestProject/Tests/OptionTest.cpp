#include "CUnit.h"
#include "Option.h"
#include "OptionsSet.h"

using namespace JaCoAP;

static void TEST_Option_OptionListIterator() {
	OptionList optionList;

	optionList.Insert(Option::Allocate(1, 4, pu8("home")));
	optionList.Insert(Option::Allocate(1, 4, pu8("hall")));
	optionList.Insert(Option::Allocate(1, 5, pu8("right")));
	optionList.Insert(Option::Allocate(1, 5, pu8("light")));

	OptionListIterator iterator = optionList.begin();

	for (; iterator != optionList.end(); ++iterator) {
		(*iterator)->Print();
	}
}

static void TEST_Option_OpenListPrint() {
	OptionList optionList;

	optionList.Insert(Option::Allocate(1, 4, pu8("home")));
	optionList.Insert(Option::Allocate(1, 4, pu8("hall")));
	optionList.Insert(Option::Allocate(1, 5, pu8("right")));
	optionList.Insert(Option::Allocate(1, 5, pu8("light")));

	optionList.Print();
}

static void TEST_Option_OpenListSortedInsert() {
	OptionList optionList;

	optionList.Insert(Option::Allocate(1, 0, nullptr));
	optionList.Insert(Option::Allocate(3, 0, nullptr));
	optionList.Insert(Option::Allocate(1, 0, nullptr));
	optionList.Insert(Option::Allocate(4, 0, nullptr));
	optionList.Insert(Option::Allocate(7, 0, nullptr));
	optionList.Insert(Option::Allocate(5, 0, nullptr));
	optionList.Insert(Option::Allocate(60, 0, nullptr));
	optionList.Insert(Option::Allocate(15, 0, nullptr));
	optionList.Insert(Option::Allocate(11, 0, nullptr));
	optionList.Insert(Option::Allocate(35, 0, nullptr));
	optionList.Insert(Option::Allocate(17, 0, nullptr));
	optionList.Insert(Option::Allocate(39, 0, nullptr));

	optionList.Print();
}

static void TEST_Option_OptionProvider() {
	OptionsSet optionProvider;

	optionProvider.AddOption(1, u64(0x41424344));
	optionProvider.AddOption(5, u32(0x4242));
	optionProvider.AddOption(34, u32(0x4343));

	optionProvider.Print();

	CU_ASSERT( optionProvider.HasOpt(34) == true );
	CU_ASSERT( optionProvider.HasOpt( 4 ) == false );
}

void TEST_SUITE_Option() {
	CU_pSuite group = CU_add_suite("Option tests", nullptr, nullptr);

	//CU_add_test( group, "OptionListIterator", TEST_Option_OptionListIterator );
	//CU_add_test( group, "OpenListSortedInsert", TEST_Option_OpenListSortedInsert );
	//CU_add_test( group, "OptionListPrint", TEST_Option_OpenListPrint );
	CU_add_test(group, "OptionProvider", TEST_Option_OptionProvider);

}
