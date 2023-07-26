#include "munit.h"
#include "helpers.h"

#include "europa.h"
#include "europa/table.h"
#include "europa/symbol.h"
#include "europa/string.h"
#include "europa/number.h"

static void* table_setup(MunitParameter params[], void* user_data) {
	europa* s;
	s = bootstrap_default_instance();
	return s;
}

void table_teardown(void* fixture) {
	europa* s;
	if (fixture == NULL)
		return;
	s = (europa*)fixture;
	terminate_default_instance(s);
}

MunitResult simple_table_test(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	struct europa_table* t;
	struct europa_symbol* k;
	eu_string* v;
	struct europa_value key, value;
	struct europa_value *rv, out;

	// create the table
	t = eutable_new(s, 0);
	munit_assert_not_null(t);

	// create a symbol
	k = eusymbol_new(s, "key-symbol");
	munit_assert_not_null(k);
	_eu_makesym(&key, k);

	// create a string
	v = eustring_new(s, "string-value");
	munit_assert_not_null(v);
	_eu_makestring(&value, v);

	// try getting a key that does not exist (yet)
	munit_assert_int(eutable_get(s, t, &key, &rv), ==, EU_RESULT_OK);
	munit_assert_null(rv);

	// add the key to the list
	munit_assert_int(eutable_create_key(s, t, &key, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);

	*rv = value; // set the value

	// try getting the value again
	munit_assert_int(eutable_get(s, t, &key, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);

	// make sure the values are equal
	munit_assert_int(euvalue_eq(rv, &value, &out), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(&out), ==, EU_TYPE_BOOLEAN);
	munit_assert_true(out.value.boolean);

	return MUNIT_OK;
}

MunitResult multiple_elements(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	struct europa_symbol *k1, *k2, *k3;
	struct europa_value key1, key2, key3, *rv;
	struct europa_table* t;

	k1 = eusymbol_new(s, "joyful");
	munit_assert_not_null(k1);
	_eu_makesym(&key1, k1);

	k2 = eusymbol_new(s, "synaphea");
	munit_assert_not_null(k2);
	_eu_makesym(&key2, k2);

	k3 = eusymbol_new(s, "dram");
	munit_assert_not_null(k3);
	_eu_makesym(&key3, k3);

	// create the table
	t = eutable_new(s, 0); // with a zero size to test growing the table

	// insert keys
	munit_assert_int(eutable_create_key(s, t, &key1, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);
	*rv = key1;
	munit_assert_int(eutable_create_key(s, t, &key2, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);
	*rv = key2;
	munit_assert_int(eutable_create_key(s, t, &key3, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);
	*rv = key3;

	// check keys
	munit_assert_int(eutable_get(s, t, &key1, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);
	munit_assert_ptr_equal(_euvalue_to_obj(rv), k1);
	munit_assert_int(eutable_get(s, t, &key2, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);
	munit_assert_ptr_equal(_euvalue_to_obj(rv), k2);
	munit_assert_int(eutable_get(s, t, &key3, &rv), ==, EU_RESULT_OK);
	munit_assert_not_null(rv);
	munit_assert_ptr_equal(_euvalue_to_obj(rv), k3);

	return MUNIT_OK;
}

MunitResult simple_index(MunitParameter params[], void* fixture) {
	europa* s = cast(europa*, fixture);
	struct europa_symbol* key;
	struct europa_table *t, *i;
	struct europa_value *vr, kv;

	// create the key
	key = eusymbol_new(s, "+!");
	munit_assert_not_null(key);
	_eu_makesym(&kv, key);

	// create the index table
	i = eutable_new(s, 1);
	munit_assert_not_null(i);
	// create the table
	t = eutable_new(s, 1);
	munit_assert_not_null(t);

	// set the index
	_eutable_set_index(t, i);

	// set key on the index
	munit_assert_int(eutable_create_key(s, i, &kv, &vr), ==, EU_RESULT_OK);
	munit_assert_not_null(vr);
	_eu_makeint(vr, 1010);
	vr = NULL;

	// try getting the value recursively
	munit_assert_int(eutable_rget(s, t, &kv, &vr), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(vr), ==, EU_TYPE_NUMBER);
	munit_assert_int(_eunum_i(vr), ==, 1010);

	// add the key to t
	munit_assert_int(eutable_create_key(s, t, &kv, &vr), ==, EU_RESULT_OK);
	munit_assert_not_null(vr);
	_eu_makeint(vr, 2020);
	vr = NULL;

	// try getting the value recursively again
	munit_assert_int(eutable_rget(s, t, &kv, &vr), ==, EU_RESULT_OK);
	munit_assert_int(_euvalue_type(vr), ==, EU_TYPE_NUMBER);
	munit_assert_int(_eunum_i(vr), ==, 2020);

	return MUNIT_OK;
}

MunitTest tabletests[] = {
	{
		"/simple",
		simple_table_test,
		table_setup,
		table_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/multiple",
		multiple_elements,
		table_setup,
		table_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/simple-index",
		simple_index,
		table_setup,
		table_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{ NULL },
};

MunitSuite tablesuite = {
	"/table",
	tabletests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE,
};
