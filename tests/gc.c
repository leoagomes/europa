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


/** Realloc-like function to be used by the garbage collector.
 * 
 * @todo Use a function provided by an auxilary library (to implement).
 */
void* rlike(void* ud, void* ptr, unsigned long long size) {
	return realloc(ptr, size);
}

/** Sets up an Europa state with a correctly intialized GC.
 * 
 * @returns An Europa state with a gc that works properly.
 */
static void* gc_setup(MunitParameter params[], void* user_data) {
	europa* s;

	/* we need to allocate memory for the state, because it tries to leave
	 * memory management to the GC
	 * 
	 * TODO: maybe sometime use something provided by an auxilary library */
	s = (europa*)malloc(sizeof(europa));
	if (s == NULL)
		return NULL;

	eugc_init(&(s->gc), NULL, rlike);
	eu_init(s);

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

	eugc_destroy(_eu_get_gc(s));
	free(s);
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
	eu_gcobj* obj = eugc_new_object(_eu_get_gc(s), EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_gcobj));

	/* make sure something was allocated */
	munit_assert_ptr_not_null(obj);

	obj->_mark = 10;
	munit_assert(obj->_mark == 10);

	return MUNIT_OK;
}

/** Tests whether the naive sweep algorithm correctly releases objects.
 * 
 * The test creates 4 objects, marks 2, does a naive sweep and verifies
 * that the only lasting objects were the marked ones.
 */
MunitResult test_gc_naive_sweep(MunitParameter params[], void* fixture) {
	europa* s;
	eu_gcobj* obj[4];
	eu_gc* gc;

	if (fixture == NULL)
		return MUNIT_ERROR;

	s = (europa*)fixture;
	gc = _eu_get_gc(s);

	/* allocate 4 objects */
	obj[0] = eugc_new_object(gc, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_gcobj));
	munit_assert_ptr_not_null(obj[0]);
	obj[1] = eugc_new_object(gc, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_gcobj));
	munit_assert_ptr_not_null(obj[1]);
	obj[2] = eugc_new_object(gc, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_gcobj));
	munit_assert_ptr_not_null(obj[2]);
	obj[3] = eugc_new_object(gc, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_gcobj));
	munit_assert_ptr_not_null(obj[3]);

	/* mark first and third objects */
	obj[0]->_mark = EUGC_COLOR_BLACK;
	obj[2]->_mark = EUGC_COLOR_BLACK;

	/* check whether objects are being properly added to the list */
	munit_assert_ptr_equal(gc->last_obj, obj[3]);
	munit_assert_ptr_equal(gc->last_obj->_next, obj[2]);
	munit_assert_ptr_equal(gc->last_obj->_next->_next, obj[1]);
	munit_assert_ptr_equal(gc->last_obj->_next->_next->_next, obj[0]);
	munit_assert_ptr_null(gc->last_obj->_next->_next->_next->_next);

	/* collect garbage objects */
	eugc_naive_sweep(gc);

	/* check if marked objects are the only ones that remain */
	munit_assert_ptr_equal(gc->last_obj, obj[2]);
	munit_assert_ptr_equal(gc->last_obj->_next, obj[0]);
	munit_assert_ptr_null(gc->last_obj->_next->_next);

	/* make sure that they are now marked white */
	munit_assert_int(gc->last_obj->_mark, ==, EUGC_COLOR_WHITE);
	munit_assert_int(gc->last_obj->_next->_mark, ==, EUGC_COLOR_WHITE);

	return MUNIT_OK;
}


MunitTest gctests[] = {
	{
		"/test-object-creation",
		test_object_creation,
		gc_setup,
		gc_teardown,
		MUNIT_TEST_OPTION_NONE,
		NULL,
	},
	{
		"/text-gc-naive-sweep",
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