/** Auxilary functions for the testing environment.
 * 
 * @file helpers.c
 * @author Leonardo G.
 * @ingroup tests
 */
#include "helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include "munit.h"

/** Realloc-like function to be used by the garbage collector.
 * 
 * @todo Use a function provided by an auxilary library (to implement).
 */
void* rlike(void* ud, void* ptr, unsigned long long size) {
	return realloc(ptr, size);
}

europa* bootstrap_default_instance(void) {
	europa* s;
	eu_result err;

	/* we need to allocate memory for the state, because it tries to leave
	 * memory management to the GC
	 * 
	 * TODO: maybe sometime use something provided by an auxilary library */
	s = europa_new(rlike, NULL, NULL, &err);
	return s;
}

void terminate_default_instance(europa* s) {
	eugc_destroy(s);
	free(s);
}