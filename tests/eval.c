#include "munit.h"
#include "suites.h"
#include "europa.h"
#include "eu_symbol.h"
#include "eu_pair.h"
#include "eu_port.h"
#include "eu_string.h"
#include "eu_rt.h"
#include "eu_number.h"
#include "eu_ccont.h"
#include "eu_error.h"

#include <stdio.h>
#include <stdlib.h>

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

	if (s->err) {
		printf("Europa state had an error: %s\n", _euerror_message(s->err));
	}

	terminate_default_instance(s);
}

/* this is "absolutely don't even care mode", and should work because it doesn't
 * do anything tricky */
eu_result simple_test_cl(europa* s) {
	_eu_makeint(_eu_acc(s), 1234);
	return EU_RESULT_OK;
}

MunitResult test_do_string(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value result;

	munit_assert_int(eu_do_string(s, "((lambda (x) x) 10)", &result), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&result), ==, EU_TYPE_NUMBER);
	munit_assert_int(result.value.i, ==, 10);

	munit_assert_int(eu_do_string(s, "(if #t #t #f)", &result), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&result), ==, EU_TYPE_BOOLEAN);
	munit_assert_true(result.value.boolean);

	munit_assert_int(eu_do_string(s, "(if #f #t #f)", &result), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&result), ==, EU_TYPE_BOOLEAN);
	munit_assert_false(result.value.boolean);

	eu_value *tv, key;
	eu_symbol* keysym;
	eu_closure* cl;

	cl = eucl_new(s, simple_test_cl, NULL, s->global->env);
	munit_assert_not_null(cl);

	keysym = eusymbol_new(s, "c-function");
	munit_assert_not_null(keysym);
	_eu_makesym(&key, keysym);

	munit_assert_int(eutable_create_key(s, s->global->env, &key, &tv), ==, EU_RESULT_OK);
	munit_assert_not_null(tv);
	_eu_makeclosure(tv, cl);

	munit_assert_int(eu_do_string(s, "(c-function)", &result), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&result), ==, EU_TYPE_NUMBER);
	munit_assert_int(result.value.i, ==, 1234);

	return EU_RESULT_OK;
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
