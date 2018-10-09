/** Testing code main file.
 * 
 * @file main.c
 * @author Leonardo G.
 * 
 * @defgroup tests Tests
 */
#include "munit.h"

#include "suites.h"

int main(int argc, char* argv[]) {
	MunitSuite suites[] = {
		gcsuite,
		readsuite,
		tablesuite,
		NULL
	};

	MunitSuite mainsuite = {
		"",
		NULL,
		suites,
		1,
		MUNIT_SUITE_OPTION_NONE,
	};
	return munit_suite_main(&mainsuite, NULL, argc, argv);
}
