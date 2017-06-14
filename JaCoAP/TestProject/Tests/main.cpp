#include "CUnit.h"
#include "Automated.h"

static void RunTests( );

int main( ) {
	CU_pFailureRecord   failureRecords = nullptr;
	CU_initialize_registry( );

	RunTests( );

	/* Run all tests using the automated interface */
	CU_set_output_filename( "./rcm" );
	CU_list_tests_to_file( );
	CU_automated_run_tests( );

	printf( "\n*** %d/%d TEST(S) FAILED ***\n", CU_get_number_of_tests_failed( ), CU_get_number_of_tests_run( ) );
	printf( "\n*** %d/%d TEST(S) FAILED ***\n", CU_get_number_of_tests_failed( ), CU_get_number_of_tests_run( ) );

	failureRecords = CU_get_failure_list( );

	while ( failureRecords ) {
		printf( "%s %d %s %s\n", failureRecords->strFileName, failureRecords->uiLineNumber, failureRecords->strCondition, failureRecords->pTest->pName );
		failureRecords = failureRecords->pNext;
	}

	CU_cleanup_registry( );
}

extern void TEST_SUITE_Option( );
extern void TEST_SUITE_IpAddress( );

void RunTests( ) {
	//TEST_SUITE_Option( );
	TEST_SUITE_IpAddress( );
}
