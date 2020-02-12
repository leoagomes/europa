/** Garbage collection related functions.
 *
 * @file gc.c
 * @author Leonardo G.
 */
#include "europa/gc.h"

#include "europa/pair.h"
#include "europa/symbol.h"
#include "europa/table.h"
#include "europa/vector.h"
#include "europa/port.h"
#include "europa/rt.h"

#include <stdio.h>

/* helper macros */
#define eugco_mark(obj) ((obj)->_color)
#define eugco_markwhite(obj) ((obj)->_color = EUGC_COLOR_WHITE)
#define eugco_markgrey(obj) ((obj)->_color = EUGC_COLOR_GREY)
#define eugco_markblack(obj) ((obj)->_color = EUGC_COLOR_BLACK)

/* forward function declarations */
int eugco_destroy(europa* s, eu_object* obj);

/* function definitions */

/** Initializes the GC structure.
 *
 * @param gc A pointer to the garbage collection structure.
 * @param ud The userdata pointer to be passed to the realloc-like function.
 * @return Whether initializing the data was successful.
 */
int eugc_init(eu_gc* gc, void* ud, eu_realloc rlc) {
	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* initialize the realloc function and user data */
	gc->realloc = rlc;
	gc->ud = ud;

	/* initializing the list of objects. */
	gc->last_obj = NULL;

	/* set up object list head */
	gc->objs_head._color = EUGC_DO_NOT_TOUCH;
	gc->objs_head._next = &(gc->objs_head);
	gc->objs_head._previous = &(gc->objs_head);

	/* set up root set head */
	gc->root_head._color = EUGC_DO_NOT_TOUCH;
	gc->root_head._next = &(gc->root_head);
	gc->root_head._previous = &(gc->root_head);

	return EU_RESULT_OK;
}

/** Destroys the GC context, collecting all objects.
 *
 * @param gc The GC structure.
 * @return The result of the operation.
 */
int eugc_destroy(europa* s) {
	eu_object* currentobj;
	eu_object* tmp;
	eu_gc* gc = _eu_gc(s);

	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* destroy all normal objects */
	currentobj = gc->objs_head._next;
	while (currentobj != &(gc->objs_head)) {
		eugco_destroy(s, currentobj);
		currentobj = currentobj->_next;
	}
	/* and root set objects */
	currentobj = gc->root_head._next;
	while (currentobj != &(gc->root_head)) {
		eugco_destroy(s, currentobj);
		currentobj = currentobj->_next;
	}

	/* then free their memories */
	currentobj = gc->objs_head._next;
	while (currentobj != &(gc->objs_head)) {
		tmp = currentobj->_next;
		_eu_checkreturn(eugc_remove_object(s, currentobj));
		_eugc_free(gc, currentobj);
		currentobj = tmp;
	}
	/* and root set objects */
	currentobj = gc->root_head._next;
	while (currentobj != &(gc->root_head)) {
		tmp = currentobj->_next;
		_eu_checkreturn(eugc_remove_object(s, currentobj));
		_eugc_free(gc, currentobj);
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
eu_object* eugc_new_object(europa* s, eu_byte type, unsigned long long size) {
	eu_object* obj;
	eu_gc* gc = _eu_gc(s);

	/* alloc object memory */
	obj = _eugc_malloc(gc, size);
	if (!obj) {
		/* if failed to allocate memory, perform a garbage collection cycle. */
		if (eugc_naive_collect(s) != EU_RESULT_OK)
			return NULL;
		obj = _eugc_malloc(gc, size); /* try again */
		if (!obj) { /* still not enough memory */
			return NULL;
		}
	}

	/* add object to object list */
	obj->_next = gc->objs_head._next;
	obj->_previous = &(gc->objs_head);
	gc->objs_head._next->_previous = obj;
	gc->objs_head._next = obj;

	eugco_markwhite(obj);

	/* initialize other fields */
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
int eugc_naive_collect(europa* s) {
	int res;
	eu_object* obj;
	eu_gc* gc = _eu_gc(s);

	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* mark all objects in root set */
	obj = gc->root_head._next;
	while (obj != &(gc->root_head)) {
		_eu_checkreturn(eugc_naive_mark(s, obj));
		obj = obj->_next;
	}

	/* sweep */
	if ((res = eugc_naive_sweep(s)))
		return res;

	return EU_RESULT_OK;
}

/** Performs a naive mark on all objects.
 *
 * The naive mark corresponds to the mark part of a naive mark and sweep
 * algorithm.
 *
 * @param gc the GC structure.
 * @param obj the object to mark.
 */
int eugc_naive_mark(europa* s, eu_object* obj) {
	eu_gc* gc = _eu_gc(s);

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
		_eu_checkreturn(eupair_mark(s, eugc_naive_mark, _euobj_to_pair(obj)));
		break;

	case EU_TYPE_VECTOR:
		_eu_checkreturn(euvector_mark(s, eugc_naive_mark, _euobj_to_vector(obj)));
		break;

	case EU_TYPE_PORT:
		_eu_checkreturn(euport_mark(s, eugc_naive_mark, _euobj_to_port(obj)));
		break;

	case EU_TYPE_TABLE:
		_eu_checkreturn(eutable_mark(s, eugc_naive_mark, _euobj_to_table(obj)));
		break;

	case EU_TYPE_CLOSURE:
		_eu_checkreturn(eucl_mark(s, eugc_naive_mark, _euobj_to_closure(obj)));
		break;

	case EU_TYPE_CONTINUATION:
		_eu_checkreturn(eucont_mark(s, eugc_naive_mark, _euobj_to_cont(obj)));
		break;

	case EU_TYPE_PROTO:
		_eu_checkreturn(euproto_mark(s, eugc_naive_mark, _euobj_to_proto(obj)));
		break;

	case EU_TYPE_STATE:
		_eu_checkreturn(eustate_mark(s, eugc_naive_mark, cast(europa*, obj)));
		break;

	case EU_TYPE_GLOBAL:
		_eu_checkreturn(euglobal_mark(s, eugc_naive_mark, cast(eu_global*, obj)));
		break;

	case EU_TYPE_USERDATA:
		break;

	/* object types that reference no other objects */
	case EU_TYPE_SYMBOL:
	case EU_TYPE_STRING:
	case EU_TYPE_BYTEVECTOR:
	default:
		break;
	}

	eugco_markblack(obj);

	return EU_RESULT_OK;
}

/**
 * @brief Performs the sweeping stage of a naive mark-and-sweep GC.
 *
 * @param s The Europa state.
 * @return The result of the operation.
 */
int eugc_naive_sweep(europa* s) {
	eu_object *current, *aux;
	int res;
	eu_gc* gc = _eu_gc(s);

	if (gc == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* start at just after the head */
	current = gc->objs_head._next;

	while (current != &(gc->objs_head)) { /* run until we've reached the head again */
		switch (current->_color) {
		/* remove objects that couldn't be reached during the mark stage */
		case EUGC_COLOR_WHITE:
			aux = current->_next; /* save the next object */

			/* remove the object from the list */
			_eu_checkreturn(eugc_remove_object(s, current));

			/* run the object's destructor */
			res = eugco_destroy(s, current);

			/* free the chunk of memory */
			_eugc_free(gc, current);

			current = aux; /* correct next and restart loop */
			continue;

		/* keep reachable objects */
		case EUGC_COLOR_BLACK:
			/* mark reachable object as white (for next cycle) */
			eugco_markwhite(current);
			break;

		/* do not touch objects with the do not touch color */
		case EUGC_DO_NOT_TOUCH:
			break;

		/* finding a grey node during sweep is an error */
		/* finding a node with a different color is also an error */
		case EUGC_COLOR_GREY:
		default:
			/* TODO: report error (return error?)*/
			break;
		}

		/* go to next object */
		current = current->_next;
	}

	return EU_RESULT_OK;
}

#define checkreturn_result(res, e) \
	if ((res = (e))) {\
		return res;\
	}

int eugco_destroy(europa* s, eu_object* obj) {
	int res;
	eu_gc* gc = _eu_gc(s);

	switch (_euobj_type(obj)) {
	/* objects that may reference external resources that need closing */
	case EU_TYPE_TABLE:
		checkreturn_result(res, eutable_destroy(s, _euobj_to_table(obj)));
		break;

	case EU_TYPE_PORT:
		checkreturn_result(res, euport_destroy(s, _euobj_to_port(obj)));
		break;

	case EU_TYPE_CONTINUATION:
		checkreturn_result(res, eucont_destroy(s, _euobj_to_cont(obj)));
		break;

	case EU_TYPE_CLOSURE:
		checkreturn_result(res, eucl_destroy(s, _euobj_to_closure(obj)));
		break;

	case EU_TYPE_PROTO:
		checkreturn_result(res, euproto_destroy(s, _euobj_to_proto(obj)));
		break;

	case EU_TYPE_PAIR:
	case EU_TYPE_USERDATA:
	case EU_TYPE_VECTOR:
		break;

	/* object types that reference no other objects */
	case EU_TYPE_SYMBOL:
	case EU_TYPE_STRING:
	case EU_TYPE_BYTEVECTOR:
	default:
		break;
	}

	return EU_RESULT_OK;
}

/**
 * @brief Removes an object from its list.
 *
 * @param s The Europa state.
 * @param obj The target object.
 * @return The result of the operation.
 */
int eugc_remove_object(europa* s, eu_object* obj) {
	/* prevent the user from breaking the list */
	if (obj == &(_eu_gc(s)->objs_head))
		return EU_RESULT_BAD_ARGUMENT;

	/* check if object is in any list */
	if (!obj->_previous || !obj->_next || obj->_previous == obj || obj->_next == obj)
		return EU_RESULT_OK;

	/* remove it from the list */
	obj->_next->_previous = obj->_previous; /* point next's previous to previous */
	obj->_previous->_next = obj->_next; /* point previous' next to next */

	/* remove its links */
	obj->_next = obj->_previous = obj;

	return EU_RESULT_OK;
}

/**
 * @brief Adds an object to the head's list.
 *
 * This function does not remove the object from its list prior to adding it to
 * the target list, so it should be removed by the user. This function also
 * touches nothing except for the next and previous pointers for both objects,
 * meaning it won't change any of the passed objects' marks or types.
 *
 * @param s The Europa state.
 * @param head The target list's head.
 * @param obj The target object.
 * @return The result of the operation.
 */
int eugc_add_object(europa* s, eu_object* head, eu_object* obj) {
	/* add object to object list */
	obj->_next = head->_next;
	obj->_previous = head;
	obj->_next->_previous = obj;
	obj->_previous->_next = obj;

	return EU_RESULT_OK;
}

/**
 * @brief Moves an object into the root set.
 *
 * This will remove an object from its list and place it at the root set. If the
 * object is already in the root set, this operation will effectively move it
 * closer to the head. This will also set the object's color back to white.
 *
 * @param s The Europa state.
 * @param obj The target object.
 * @return The result of the operation.
 */
int eugc_move_to_root(europa* s, eu_object* obj) {
	/* remove object from previous list */
	_eu_checkreturn(eugc_remove_object(s, obj));
	/* add object to root list */
	_eu_checkreturn(eugc_add_object(s, _eugc_root_head(_eu_gc(s)), obj));
	/* mark it white */
	eugco_markwhite(obj);

	return EU_RESULT_OK;
}

/**
 * @brief Moves an object from the root set into the object list.
 *
 * This function removes the object from its list and moves it into the "normal"
 * object list (the list traversed during sweeping phase). If the object is
 * already in the object list, it will just be moved closer to the list's head.
 * This also turns the object back into white.
 *
 * @param s The Europa state.
 * @param obj The target object.
 * @return The result of the operation.
 */
int eugc_move_off_root(europa* s, eu_object* obj) {
	/* remove object from previous list */
	_eu_checkreturn(eugc_remove_object(s, obj));
	/* add it to the object list */
	_eu_checkreturn(eugc_add_object(s, _eugc_objs_head(_eu_gc(s)), obj));
	/* paint it white */
	eugco_markwhite(obj);

	return EU_RESULT_OK;
}
