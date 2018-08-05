#include "munit.h"

#include "suites.h"

int main(int argc, char* argv[]) {
	MunitSuite suites[] = {
		gcsuite,
		{0},
	};

	MunitSuite mainsuite = {
		"/",
		NULL,
		suites,
		1,
		MUNIT_SUITE_OPTION_NONE,
	};

	return munit_suite_main(&mainsuite, NULL, argc, argv);
}
