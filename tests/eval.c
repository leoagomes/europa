#include "munit.h"
#include "suites.h"
#include "helpers.h"
#include "europa.h"

#include "eu_commons.h"
#include "eu_symbol.h"
#include "eu_pair.h"
#include "eu_port.h"
#include "eu_string.h"
#include "eu_rt.h"
#include "eu_number.h"
#include "eu_ccont.h"
#include "eu_error.h"
#include "eu_vector.h"
#include "eu_bytevector.h"

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

MunitResult test_lambda_formals(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value result;

	assert_ok(eu_do_string(s, "((lambda (a b) b) 123 456)", &result));
	assertv_int(&result, ==, 456);

	assert_ok(eu_do_string(s, "((lambda x x) 123 456)", &result));
	assertv_type(&result, EU_TYPE_PAIR);
	munit_assert_not_null(_euvalue_to_obj(&result));
	assertv_type(_eupair_head(_euvalue_to_pair(&result)), EU_TYPE_NUMBER);
	assertv_int(_eupair_head(_euvalue_to_pair(&result)), ==, 123);
	assertv_type(_eupair_tail(_euvalue_to_pair(&result)), EU_TYPE_PAIR);
	assertv_type(_eupair_head(_euvalue_to_pair(_eupair_tail(_euvalue_to_pair(&result)))),
		EU_TYPE_NUMBER);
	assertv_int(_eupair_head(_euvalue_to_pair(_eupair_tail(_euvalue_to_pair(&result)))),
		==, 456);

	assert_ok(eu_do_string(s, "((lambda (a b . c) c) 1 2 3 4)", &result));
	assertv_type(&result, EU_TYPE_PAIR);
	assertv_int(_eupair_head(_euvalue_to_pair(&result)), ==, 3);
	assertv_type(_eupair_tail(_euvalue_to_pair(&result)), EU_TYPE_PAIR);
	assertv_int(_eupair_head(_euvalue_to_pair(_eupair_tail(_euvalue_to_pair(&result)))), ==, 4);
	assertv_type(_eupair_tail(_euvalue_to_pair(_eupair_tail(_euvalue_to_pair(&result)))), EU_TYPE_NULL);

	assert_ok(eu_do_string(s, "((lambda (a b . c) b) 1 2 3 4)", &result));
	assertv_type(&result, EU_TYPE_NUMBER);
	assertv_int(&result, ==, 2);

	return MUNIT_OK;
}

MunitResult test_eval_constants(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value out;

	assert_ok(eu_do_string(s, "1234", &out));
	assertv_type(&out, EU_TYPE_NUMBER);
	assertv_int(&out, ==, 1234);

	assert_ok(eu_do_string(s, "12.34", &out));
	assertv_type(&out, EU_TYPE_NUMBER);
	assertv_real(&out, ==, 12.34);

	assert_ok(eu_do_string(s, "#t", &out));
	assertv_type(&out, EU_TYPE_BOOLEAN);
	assertv_true(&out);

	assert_ok(eu_do_string(s, "#f", &out));
	assertv_type(&out, EU_TYPE_BOOLEAN);
	assertv_false(&out);

	assert_ok(eu_do_string(s, "#\\newline", &out));
	assertv_type(&out, EU_TYPE_CHARACTER);
	assertv_char(&out, ==, '\n');

#define STRING_TEXT "a piece of text"
	assert_ok(eu_do_string(s, "\"" STRING_TEXT "\"", &out));
	assertv_type(&out, EU_TYPE_STRING);
	assertv_string_equal(&out, STRING_TEXT);

#define SYMBOL_TEXT "thank-you-kanye-very-cool"
	assert_ok(eu_do_string(s, "'" SYMBOL_TEXT, &out));
	assertv_type(&out, EU_TYPE_SYMBOL);
	assertv_symbol_equal(&out, SYMBOL_TEXT);

	assert_ok(eu_do_string(s, "#(123 a)", &out));
	assertv_type(&out, EU_TYPE_VECTOR);
	munit_assert_int(_euvector_length(_euvalue_to_vector(&out)), ==, 2);
	assertv_type(_euvector_ref(_euvalue_to_vector(&out), 0), EU_TYPE_NUMBER);
	assertv_int(_euvector_ref(_euvalue_to_vector(&out), 0), ==, 123);
	assertv_type(_euvector_ref(_euvalue_to_vector(&out), 1), EU_TYPE_SYMBOL);
	assertv_symbol_equal(_euvector_ref(_euvalue_to_vector(&out), 1), "a");

	assert_ok(eu_do_string(s, "#u8(0 255)", &out));
	assertv_type(&out, EU_TYPE_BYTEVECTOR);
	munit_assert_int(_eubvector_length(_euvalue_to_bvector(&out)),==,2);
	munit_assert_uint8(_eubvector_ref(_euvalue_to_bvector(&out), 0), ==, 0);
	munit_assert_uint8(_eubvector_ref(_euvalue_to_bvector(&out), 1), ==, 255);

	return MUNIT_OK;
}

MunitResult test_eval_ifs(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value result;

	assert_ok(eu_do_string(s, "(if #t #t #f)", &result));
	assertv_type(&result, EU_TYPE_BOOLEAN);
	assertv_true(&result);

	assert_ok(eu_do_string(s, "(if #f #t #f)", &result));
	assertv_type(&result, EU_TYPE_BOOLEAN);
	assertv_false(&result);

	return MUNIT_OK;
}

/*
 * C functions that don't call into Europa code or that can't have their
 * continuations captured need no tagging.
 */
eu_result simple_test_cl(europa* s) {
	_eu_makeint(_eu_acc(s), 1234);
	return EU_RESULT_OK;
}

eu_result cl_that_calls(europa* s) {
	eu_value v;

	_eucc_dispatcher(s,
		_eucc_dtag(argument_call)
	);

	_eucc_tag(s, argument_call,
		_eu_checkreturn(eucc_frame(s));
		_eu_checkreturn(eu_do_string(s, "1234", NULL));
	)

	v = *_eu_acc(s);
	_eu_makeint(_eu_acc(s), _eunum_i(&v));

	return EU_RESULT_OK;
}

MunitResult test_c_closures(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value *tv, key, result;
	eu_symbol* keysym;
	eu_closure* cl;

	cl = eucl_new(s, simple_test_cl, NULL, s->global->env);
	munit_assert_not_null(cl);

	keysym = eusymbol_new(s, "c-function");
	munit_assert_not_null(keysym);
	_eu_makesym(&key, keysym);

	assert_ok(eutable_create_key(s, s->global->env, &key, &tv));
	munit_assert_not_null(tv);
	_eu_makeclosure(tv, cl);

	assert_ok(eu_do_string(s, "(c-function)", &result));
	assertv_type(&result, EU_TYPE_NUMBER);
	assertv_int(&result, ==, 1234);

	cl->cf = cl_that_calls;

	assert_ok(eu_do_string(s, "(c-function)", &result));
	assertv_type(&result, EU_TYPE_NUMBER);
	assertv_int(&result, ==, 1234);

	return MUNIT_OK;
}

MunitResult test_simple_callcc(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	eu_value result;

	assert_ok(eu_do_string(s,
		"((lambda (value) (call/cc (lambda (return) (return value)))) 123)", &result));
	assertv_type(&result, EU_TYPE_NUMBER);
	assertv_int(&result, ==, 123);

	assert_ok(eu_do_string(s,
		"((lambda (c) (set! c (call/cc (lambda (i) i))) (if c (c #f) 1234)) #t)",
		&result));
	assertv_type(&result, EU_TYPE_NUMBER);
	assertv_int(&result, ==, 1234);

	return MUNIT_OK;
}

MunitTest evaltests[] = {
	{
		"/constants",
		test_eval_constants,
		eval_setup,
		eval_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/lambda-formals",
		test_lambda_formals,
		eval_setup,
		eval_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/if",
		test_eval_ifs,
		eval_setup,
		eval_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/c-closures",
		test_c_closures,
		eval_setup,
		eval_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/call-cc",
		test_simple_callcc,
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
