/** GC module testing file.
 * 
 * @file gc.c
 * @author Leonardo G.
 * @ingroup tests
 */
#include <stdlib.h>
#include <stdio.h>

#include "munit.h"
#include "suites.h"
#include "helpers.h"

#include "europa.h"

/* This file holds tests for the garbage collection facilities of the language.
 * List of tests:
 * 
 * - [x] object creation (eugc_new_object)
 * 
 * - [ ] naive mark (eugc_naive_mark)
 * - [x] naive sweep (eugc_naive_sweep)
 * - [ ] naive collect (eugc_naive_collect)
 *
 * The tests also test mark and destroy functions for primitive types:
 * 
 * - [ ] pair
 * - [ ] symbol
 */

/** Sets up an Europa state with a correctly intialized GC.
 * 
 * @returns An Europa state with a gc that works properly.
 */
static void* gc_setup(MunitParameter params[], void* user_data) {
	europa* s;

	s = bootstrap_default_instance();

	return s;
}

/** Properly tears down a gc test.
 * 
 * At the moment this only destroys the europa state originally provided.
 */
void gc_teardown(void* fixture) {
	europa* s;

	if (fixture == NULL)
		return;

	s = (europa*)fixture;

	terminate_default_instance(s);
}

/** Tests whether the GC can allocate memory for an object.
 * 
 * It creates a new managed block and tries to write something to it and read
 * from it.
 */
MunitResult test_object_creation(MunitParameter params[], void* fixture) {
	europa* s;

	if (fixture == NULL)
		return MUNIT_ERROR;

	s = (europa*)fixture;

	/* allocate an object */
	eu_object* obj = eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_object));

	/* make sure something was allocated */
	munit_assert_ptr_not_null(obj);

	obj->_mark = 10;
	munit_assert(obj->_mark == 10);

	return MUNIT_OK;
}

int object_in_list(europa* s, eu_object* obj) {
	eu_gc* gc;
	eu_object* current;

	gc = _eu_gc(s);

	current = gc->objs_head._next;
	while (current != &(gc->objs_head)) {
		if (obj == current)
			return 1;
		current = current->_next;
	}

	return 0;
}

/** Tests whether the naive sweep algorithm correctly releases objects.
 * 
 * The test creates 4 objects, marks 2, does a naive sweep and verifies
 * that the only lasting objects were the marked ones.
 */
MunitResult test_gc_naive_sweep(MunitParameter params[], void* fixture) {
	europa* s;
	eu_object* obj[4];
	eu_gc* gc;
	int i;

	if (fixture == NULL)
		return MUNIT_ERROR;

	s = (europa*)fixture;
	gc = _eu_gc(s);

	/* allocate 4 objects */
	obj[0] = eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_object));
	munit_assert_ptr_not_null(obj[0]);
	obj[1] = eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_object));
	munit_assert_ptr_not_null(obj[1]);
	obj[2] = eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_object));
	munit_assert_ptr_not_null(obj[2]);
	obj[3] = eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_object));
	munit_assert_ptr_not_null(obj[3]);

	/* mark first and third objects */
	obj[0]->_mark = EUGC_COLOR_BLACK;
	obj[2]->_mark = EUGC_COLOR_BLACK;

	/* make sure all objects are in the object list */
	for (int i = 0; i < 4; i++) {
		munit_assert_true(object_in_list(s, obj[i]));
	}

	/* collect */
	munit_assert_int(eugc_naive_sweep(s), ==, EU_RESULT_OK);

	/* make sure some objects are on the list and some aren't */
	munit_assert_true(object_in_list(s, obj[0]));
	munit_assert_true(object_in_list(s, obj[2]));
	munit_assert_false(object_in_list(s, obj[1]));
	munit_assert_false(object_in_list(s, obj[3]));

	return MUNIT_OK;
}

MunitTest gctests[] = {
	{
		"/object-creation",
		test_object_creation,
		gc_setup,
		gc_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/naive-mark-sweep",
		test_gc_naive_sweep,
		gc_setup,
		gc_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

MunitSuite gcsuite = {
	"/gc",
	gctests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE,
};