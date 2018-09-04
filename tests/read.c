/** first reader testing module
 * 
 * @file read.c
 * @author Leonardo G.
 * @ingroup tests
 */
#include <stdlib.h>
#include <stdio.h>

#include "munit.h"
#include "suites.h"
#include "helpers.h"

#include "eu_number.h"
#include "europa.h"
#include "eu_port.h"
#include "ports/eu_mport.h"
#include "ports/eu_fport.h"

static void* read_setup(MunitParameter params[], void* user_data) {
	europa* s;

	s = bootstrap_default_instance();

	return s;
}

static void read_teardown(void* fixture) {
	europa* s;
	s = (europa*)fixture;
	if (s)
		free(s);
}

MunitResult test_read_booleans(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_result res;
	eu_value out;

	eu_mport* port;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"#t #true #tru #truest");

	/* #t */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_BOOLEAN);
	munit_assert_int(out.value.boolean, ==, EU_TRUE);

	/* #true */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_BOOLEAN);
	munit_assert_int(out.value.boolean, ==, EU_TRUE);

	/* #tru */
	munit_assert_int(euport_read(s, port, &out), !=, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_ERROR);

	/* #truest */
	munit_assert_int(euport_read(s, port, &out), !=, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_ERROR);

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"#f #false #fals #falsest");

	/* #f */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_BOOLEAN);
	munit_assert_int(out.value.boolean, ==, EU_FALSE);

	/* #false */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_BOOLEAN);
	munit_assert_int(out.value.boolean, ==, EU_FALSE);

	/* #fals */
	munit_assert_int(euport_read(s, port, &out), !=, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_ERROR);

	/* #falsest */
	munit_assert_int(euport_read(s, port, &out), !=, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_ERROR);

	/* let the gc handle the ports */

	return MUNIT_OK;
}

MunitResult test_read_num_binary(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_mport* port;
	eu_value out;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"#b1001 #b#e1001 #b-1001 #b#i-1001 #b#i1001 #b1001.1");
	munit_assert_not_null(port);

	/* #b1001 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 9);

	/* #b#e1001 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 9);

	/* #b-1001 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, -9);

	/* #b#i-1001 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, -9.0);

	/* #b#i1001 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 9.0);

	/* #b1001.1 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 9.5);

	return MUNIT_OK;
}

MunitResult test_read_num_octal(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_mport* port;
	eu_value out;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"#o14 #o#e-14 #o#i14 #o14.4 #o-14.4");
	munit_assert_not_null(port);

	/* #o14 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 12);

	/* #o#e-14 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, -12);

	/* #o#i14 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 12.0);

	/* #o14.4 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 12.5);

	/* #o-14.4 */
	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, -12.5);

	return MUNIT_OK;
}

MunitResult test_read_num_dec(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_mport* port;
	eu_value out;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"123 -123 #d123 #e-123 #i123 123.45 #d#i123.45");
	munit_assert_not_null(port);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 123);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, -123);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 123);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, -123);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 123.0);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 123.45);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 123.45);

	return MUNIT_OK;
}

MunitResult test_read_num_hex(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_mport* port;
	eu_value out;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"#xaa #x#eaa #X#iAA #xaA.8");
	munit_assert_not_null(port);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 170);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER);
	munit_assert_int(out.value.i, ==, 170);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 170.0);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_NUMBER | EU_NUMBER_REAL);
	munit_assert_double(out.value.r, ==, 170.5);

	return MUNIT_OK;
}

MunitResult test_read_char(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_mport* port;
	eu_value out;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"#\\a #\\newline #\\x35 #\\invalid");
	munit_assert_not_null(port);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_CHARACTER);
	munit_assert_int(out.value.character, ==, 'a');

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_CHARACTER);
	munit_assert_int(out.value.character, ==, '\n');

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_CHARACTER);
	munit_assert_int(out.value.character, ==, '5');

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_ERROR);
	munit_assert_int(out.type, ==, EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE);

	return MUNIT_OK;
}

#define BIG_STRING "biiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"\
	"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiig"

MunitResult test_read_string(MunitParameter params[], void* fixture) {
	europa* s = (europa*)fixture;
	eu_mport* port;
	eu_value out;

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"\"small simple string\" \"escaped \\x35;\\n\"");
	munit_assert_not_null(port);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE);
	munit_assert_string_equal(eustring_text(_euobj_to_string(out.value.object)),
		"small simple string");

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE);
	munit_assert_string_equal(eustring_text(_euobj_to_string(out.value.object)),
		"escaped 5\n");

	port = eumport_from_str(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT,
		"\"" BIG_STRING "\"");
	munit_assert_not_null(port);

	munit_assert_int(euport_read(s, port, &out), ==, EU_RESULT_OK);
	munit_assert_int(out.type, ==, EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE);
	munit_assert_string_equal(eustring_text(_euobj_to_string(out.value.object)),
		BIG_STRING);
	return MUNIT_OK;
}

MunitTest readtests[] = {
	{
		"/boolean",
		test_read_booleans,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/binary-number",
		test_read_num_binary,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/octal-number",
		test_read_num_octal,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/hex-number",
		test_read_num_hex,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/decimal-number",
		test_read_num_dec,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/char",
		test_read_char,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/string",
		test_read_string,
		read_setup,
		read_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

MunitSuite readsuite = {
	"/read",
	readtests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE,
};