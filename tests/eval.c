#include "munit.h"
#include "suites.h"
#include "europa.h"
#include "eu_symbol.h"
#include "eu_pair.h"
#include "eu_port.h"
#include "eu_string.h"



static void* eval_setup(MunitParameter params[], void* user_data) {
	europa* s;

	s = bootstrap_default_instance();

	return s;
}

void eval_teardown(void* fixture) {
	europa* s;

	if (fixture == NULL)
		return;

	s = (europa*)fixture;
	terminate_default_instance(s);
}

MunitResult test_do_string(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value result;

	munit_assert_int(eu_do_string(s, "((lambda (x) x) 10)", &result));
}

MunitTest evaltests[] = {
	{
		"/do-string",
		test_do_string,
		eval_setup,
		eval_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

MunitSuite evalsuite = {
	"/eval",
	evaltests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE,
};
