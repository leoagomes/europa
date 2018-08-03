#ifndef __EUROPA_GC_H__
#define __EUROPA_GC_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

/** The realloc-like function provided to the garbage collector. */
typedef void* (*eu_realloc)(void* ud, void* ptr, size_t amount);

/** GC structure definition */
typedef struct europa_gc eu_gc;

/** The function an object should call to mark its references. */
typedef eu_result (*eu_gcmark)(eu_gc* gc, eu_gcobj* obj);

/** Possible object colors during garbage collection. */
enum eugc_color {
	EUGC_COLOR_WHITE = 0, /* object not marked for collection */
	EUGC_COLOR_GREY, /* object currently marked, but references not */
	EUGC_COLOR_BLACK /* object marked and references marked */
};

/** the marked flag */
#define EUGC_MARK (1 << 7)
/** the mask for the color part of the mark */
#define EUGC_COLOR_MASK (0xFF ^ (EUGC_MARK))

/** The garbage collector structure.
 *
 * This is the structure that holds the data used to manage garbage collection.
 */
struct europa_gc {
	void* ud; /*!< user data passed to the realloc-like function */
	eu_realloc realloc; /*!< the realloc-like function */

	eu_gcobj* last_obj; /*!< the last object created by the garbage collector */
};

/* function declarations */

eu_gcobj* eugc_new_object(eu_gc* gc, eu_byte type, size_t size);

eu_result eugc_naive_collect(eu_gc* gc, eu_gcobj* root);

eu_result eugc_naive_mark(eu_gc* gc, eu_gcobj* root);
eu_result eugc_naive_sweep(eu_gc* gc);

#endif /* __EUROPA_GC_H__ */
