#include "munit.h"

#include "suites.h"

int main(int argc, char* argv[]) {
	return munit_suite_main(&gcsuite, NULL, argc, argv);
}
