#ifndef __EUROPA_GC_H__
#define __EUROPA_GC_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

/** The realloc-like function provided to the garbage collector. */
typedef void* (*eu_realloc)(void*, void*, unsigned long long);

/** GC structure definition */
typedef struct europa_gc eu_gc;

typedef struct europa europa;

/** The function an object should call to mark its references. */
typedef eu_result (*eu_gcmark)(europa* s, eu_object* obj);

/** Possible object colors during garbage collection. */
enum eugc_color {
	EUGC_COLOR_WHITE = 0, /* object not marked for collection */
	EUGC_COLOR_GREY, /* object currently marked, but references not */
	EUGC_COLOR_BLACK, /* object marked and references marked */
	EUGC_DO_NOT_TOUCH, /* should absolutely not touch and leave to the destructor */
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

	eu_object* last_obj; /*!< the last object created by the garbage collector */

	eu_object root_head;
	eu_object objs_head; /*!< the circular object list's head */

	eu_object* root_set; /*!< list of root objects */
};

/* helper macros to translate semantically to stdlib functions */
#define eugc_malloc(gc,s) ((gc)->realloc((gc)->ud, NULL, (s)))
#define eugc_realloc(gc,ptr,s) ((gc)->realloc((gc)->ud, (ptr), (s)))
#define eugc_free(gc,ptr) ((gc)->realloc((gc)->ud, (ptr), 0))

/* function declarations */
eu_result eugc_init(eu_gc* gc, void* ud, eu_realloc rlc);
eu_result eugc_destroy(europa* s);

eu_object* eugc_new_object(europa* s, eu_byte type, unsigned long long size);

eu_result eugc_own(europa* s, eu_object* obj);
eu_result eugc_give(europa* s, eu_object* obj);

eu_result eugc_add_to_root_set(europa* s, eu_object* obj);

/* naive mark and sweep */
eu_result eugc_naive_collect(europa* s, eu_object* root);
eu_result eugc_naive_mark(europa* s, eu_object* root);
eu_result eugc_naive_sweep(europa* s);

#endif /* __EUROPA_GC_H__ */
