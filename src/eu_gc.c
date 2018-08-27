#include "eu_gc.h"

#include "eu_pair.h"
#include "eu_symbol.h"

/* forward function declarations */
eu_result eugco_destroy(eu_gc* gc, eu_gcobj* obj);

/* function definitions */

/** Initializes the GC structure.
 * 
 * @param gc A pointer to the garbage collection structure.
 * @param ud The userdata pointer to be passed to the realloc-like function.
 * @return Whether initializing the data was successful.
 */
eu_result eugc_init(eu_gc* gc, void* ud, eu_realloc rlc) {
	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	gc->realloc = rlc;
	gc->ud = ud;

	/* initializing the list of objects. */
	gc->last_obj = NULL;

	return EU_RESULT_OK;
}

/** Destroys the GC context, collecting all objects.
 * 
 * @param gc The GC structure.
 * @return The result of the operation.
 */
eu_result eugc_destroy(eu_gc* gc) {
	eu_gcobj* currentobj;
	eu_gcobj* tmp;

	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* destroy all objects */
	currentobj = gc->last_obj;
	while (currentobj != NULL) {
		eugco_destroy(gc, currentobj);
		currentobj = currentobj->_next;
	}

	/* then free their memories */
	currentobj = gc->last_obj;
	while (currentobj != NULL) {
		tmp = currentobj->_next;
		eugc_free(gc, currentobj);
		currentobj = tmp;
	}

	return EU_RESULT_OK;
}

/** Returns a new gc object of a given size.
 * 
 * Allocates an object of a given size and initializes the object's header.
 * 
 * @param gc The garbage collector structure.
 * @param type The type of the new 
 */
eu_gcobj* eugc_new_object(eu_gc* gc, eu_byte type, unsigned long long size) {
	eu_gcobj* obj;

	obj = eugc_malloc(gc, size);
	if (!obj)
		return NULL;

	obj->_next = gc->last_obj;
	gc->last_obj = obj;
	obj->_mark = EUGC_COLOR_WHITE;

	obj->_type = type;

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
	if ((res = eugc_naive_mark(gc, root)))
		return res;

	/* sweep */
	if ((res = eugc_naive_sweep(gc)))
		return res;

	return EU_RESULT_OK;
}

#define eugco_markwhite(obj) ((obj)->_mark = EUGC_COLOR_WHITE)
#define eugco_markgrey(obj) ((obj)->_mark = EUGC_COLOR_GREY)
#define eugco_markblack(obj) ((obj)->_mark = EUGC_COLOR_BLACK)
#define eugco_mark(obj) ((obj)->_mark)

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

	/* skip current object if it isn't white */
	if (eugco_mark(obj) != EUGC_COLOR_WHITE)
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
		switch (current->_mark) {
		/* remove objects that couldn't be reached during the mark stage */
		case EUGC_COLOR_WHITE:
			/* point the last node's "next" to the current's next, removing it
			 * from the object list */
			*last_next = current->_next;

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
			last_next = &(current->_next);

			/* go to next object */
			current = current->_next;
			break;

		/* finding a grey node during sweep is an error */
		/* finding a node with a different color is also an error */
		case EUGC_COLOR_GREY:
		default:
			current = current->_next;
			/* TODO: report error (return error?)*/
			break;
		}
	}

	return EU_RESULT_OK;
}

#define checkreturn_result(res, e) \
	if ((res = (e))) {\
		return res;\
	}

eu_result eugco_destroy(eu_gc* gc, eu_gcobj* obj) {
	eu_result res;

	switch (obj->_type) {
	case EU_TYPE_PAIR:
		checkreturn_result(res, eupair_destroy(gc, _euobj_to_pair(obj)))
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

	return EU_RESULT_OK;
}
