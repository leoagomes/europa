#include <stdlib.h>
#include <stdio.h>

#include "munit.h"
#include "suites.h"

#include "eu_gc.h"
#include "eu_object.h"

void* rlike(void* ud, void* ptr, size_t size) {
	return realloc(ptr, size);
}

static void* gc_setup(MunitParameter params[], void* user_data) {
	europa* s;

	s = (europa*)malloc(sizeof(europa));
	if (s == NULL)
		return NULL;

	eugc_init(&(s->gc), NULL, rlike);
	eu_init(s);

	return s;
}

void gc_teardown(void* fixture) {
	europa* s;

	if (fixture == NULL)
		return;

	s = (europa*)fixture;

	eugc_destroy(eu_get_gc(s));
	free(s);
}

MunitResult test_object_creation(MunitParameter params[], void* fixture) {
	europa* s;

	if (fixture == NULL)
		return MUNIT_ERROR;

	s = (europa*)fixture;

	/* allocate an object */
	eu_gcobj* obj = eugc_new_object(eu_get_gc(s), EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_gcobj));

	/* make sure something was allocated */
	munit_assert_ptr_not_null(obj);

	return MUNIT_OK;
}

MunitResult test_gc_naive_sweep(MunitParameter params[], void* fixture) {
	europa* s;
	eu_gcobj* obj[4];
	eu_gc* gc;

	if (fixture == NULL)
		return MUNIT_ERROR;

	s = (europa*)fixture;
	gc = eu_get_gc(s);

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
	obj[0]->mark = EUGC_COLOR_BLACK;
	obj[2]->mark = EUGC_COLOR_BLACK;

	/* check whether objects are being properly added to the list */
	munit_assert_ptr_equal(gc->last_obj, obj[3]);
	munit_assert_ptr_equal(gc->last_obj->next, obj[2]);
	munit_assert_ptr_equal(gc->last_obj->next->next, obj[1]);
	munit_assert_ptr_equal(gc->last_obj->next->next->next, obj[0]);
	munit_assert_ptr_null(gc->last_obj->next->next->next->next);

	/* collect garbage objects */
	eugc_naive_sweep(gc);

	/* check if marked objects are the only ones that remain */
	munit_assert_ptr_equal(gc->last_obj, obj[2]);
	munit_assert_ptr_equal(gc->last_obj->next, obj[0]);
	munit_assert_ptr_null(gc->last_obj->next->next);

	/* make sure that they are now marked white */
	munit_assert_int(gc->last_obj->mark, ==, EUGC_COLOR_WHITE);
	munit_assert_int(gc->last_obj->next->mark, ==, EUGC_COLOR_WHITE);

	return MUNIT_OK;
}


MunitTest gctests[] = {
	{
		"/test-object-creation",
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
	}
	{0},
};

MunitSuite gcsuite = {
	"/gc",
};