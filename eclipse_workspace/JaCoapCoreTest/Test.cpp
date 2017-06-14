#include <cute_runner.h>
#include <ide_listener.h>
#include <xml_listener.h>

#include "src/Block2InfoSuite.h"
#include "src/ByteArrayTestSuite.h"


void runSuite(int argc, char const *argv[]){
	cute::xml_file_opener xmlfile(argc,argv);
	cute::xml_listener<cute::ide_listener<>  > lis(xmlfile.out);
	cute::suite s=make_suite_Block2InfoSuite();
	s+= make_suite_ByteArrayTestSuite();
	cute::makeRunner(lis,argc,argv)(s, "Block2InfoSuite");
}

int main(int argc, char const *argv[]){
    runSuite(argc,argv);
}



