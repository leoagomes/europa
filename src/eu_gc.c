#include "eu_gc.h"

#include "eu_pair.h"


/* helper macros to translate semantically to stdlib functions */
#define eugc_malloc(gc,s) ((gc)->realloc((gc)->ud, NULL, (s)))
#define eugc_realloc(gc,ptr,s) ((gc)->realloc((gc)->ud, (ptr), (s)))
#define eugc_free(gc,ptr) ((gc)->realloc((gc)->ud, (ptr), 0))

/* forward function declarations */
eu_result eugco_destroy(eu_gc* gc, eu_gcobj* obj);

/* function definitions */

/** Returns a new gc object of a given size.
 * 
 * Allocates an object of a given size and initializes the object's header.
 * 
 * @param gc The garbage collector structure.
 * @param type The type of the new 
 */
eu_gcobj* eugc_new_object(eu_gc* gc, eu_byte type, size_t size) {
	eu_gcobj* obj;

	obj = eugc_malloc(gc, size);
	if (!obj)
		return NULL;

	obj->next = gc->last_obj;
	gc->last_obj = obj;
	obj->mark = EUGC_COLOR_WHITE;

	obj->type = type;

	return obj;
}

/** Performs a complete cycle of garbage collection. (Naive mark-and-sweep)
 * 
 * Calling this function stops the world and performs a complete garbage
 * collection cycle using a naive mark-and-sweep approach.
 * 
 * @param gc The garbage collector structure.
 * @param root The object from which to start marking.
 * @return A result pertaining to the garbage collection process.
 */
eu_result eugc_naive_collect(eu_gc* gc, eu_gcobj* root) {
	eu_result res;

	if (gc == NULL || root == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* mark */
	if ((res = eugc_mark(gc, root)))
		return res;

	/* sweep */
	if ((res = eugc_sweep(gc)))
		return res;

	return EU_RESULT_OK;
}

#define eugco_markwhite(obj) ((obj)->mark = EUGC_COLOR_WHITE)
#define eugco_markgrey(obj) ((obj)->mark = EUGC_COLOR_GREY)
#define eugco_markblack(obj) ((obj)->mark = EUGC_COLOR_BLACK)

/** Performs a naive mark on all objects.
 * 
 * The naive mark corresponds to the mark part of a naive mark and sweep
 * algorithm.
 * 
 * @param gc the GC structure.
 * @param obj the object to mark.
 */
eu_result eugc_naive_mark(eu_gc* gc, eu_gcobj* obj) {

	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	if (obj == NULL)
		return EU_RESULT_OK;

	/* mark current object grey */
	eugco_markgrey(obj);

	/* run the object's references based on its types */
	switch (_euobj_type(obj)) {
	/* object types that may need to mark refered objects */

	case EU_TYPE_PAIR:
		eupair_mark(gc, eugc_naive_mark, _euobj_to_pair(obj));
		break;

	case EU_TYPE_USERDATA:
	case EU_TYPE_VECTOR:
	case EU_TYPE_ENVIRONMENT:
	case EU_TYPE_PROCEDURE:
		break;

	/* object types that reference no other objects */
	case EU_TYPE_SYMBOL:
	case EU_TYPE_STRING:
	case EU_TYPE_BYTEVECTOR:
	case EU_TYPE_EXCEPTION:
	case EU_TYPE_PORT:
	default:
		break;
	}

	eugco_markblack(obj);

	return EU_RESULT_OK;
}

eu_result eugc_naive_sweep(eu_gc* gc) {
	eu_gcobj *current, **last_next;
	eu_result res;

	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	current = gc->last_obj;
	last_next = &(gc->last_obj);

	while (current != NULL) {
		switch (current->mark) {
		/* remove objects that couldn't be reached during the mark stage */
		case EUGC_COLOR_WHITE:
			/* point the last node's "next" to the current's next, removing it
			 * from the object list */
			*last_next = current->next;

			/* run the object's destructor */
			res = eugco_destroy(gc, current);

			/* free the chunk of memory */
			eugc_free(gc, current);

			/* go to next object */
			current = *last_next;
			break;

		/* keep reachable objects */
		case EUGC_COLOR_BLACK:
			/* mark reachable object as white (for next cycle) */
			eugco_markwhite(current);
			/* save the current "next" as the place to put the address of the
			 * next black-turned-white object in the object list */
			last_next = &(current->next);

			/* go to next object */
			current = current->next;
			break;

		/* finding a grey node during sweep is an error */
		/* finding a node with a different color is also an error */
		case EUGC_COLOR_GREY:
		default:
			current = current->next;
			/* TODO: report error (return error?)*/
			break;
		}
	}

	return EU_RESULT_OK;
}

eu_result eugco_destroy(eu_gc* gc, eu_gcobj* obj) {
}
