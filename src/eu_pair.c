/**
 * @file eu_pair.c
 * @brief Pair (cons cell) related operations.
 * @author Leonardo G.
 */
#include "eu_pair.h"

#include "eu_gc.h"

/** Creates a new (garbage collected) pair.
 * 
 * @param s the Europa state.
 * @param head the pointer to a value structure which will be copied to the
 * pair's head (car).
 * @param tail the pointer to a value structure which will be copied to the
 * pair's tail (cdr).
 * @return the newly allocated pair.
 */
eu_pair* eupair_new(europa* s, eu_value* head, eu_value* tail) {
	eu_gcobj* obj;
	eu_pair* pair;

	pair = cast(eu_pair*,eugc_new_object(s->gc, EU_TYPE_PAIR, sizeof(eu_pair)));
	if (pair == NULL)
		return NULL;

	pair->head = *head;
	pair->tail = *tail;

	return pair;
}

/** Calls the garbage collector's mark function on the pair's fields.
 * 
 * @param gc the garbage collector structure.
 * @param pair the pair to process.
 * @result the result of running the procedure.
 */
eu_result eupair_mark(eu_gc* gc, eu_gcmark mark, eu_pair* pair) {
	eu_result res;

	if (gc == NULL || pair == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* try and mark head */
	if (_euvalue_is_collectable(&(pair->head))) {
		if ((res = mark(gc, _euvalue_to_obj(&(pair->head)))))
			return res;
	}

	/* try and mark tail */
	if (_euvalue_is_collectable(&(pair->tail))) {
		if ((res = mark(gc, _euvalue_to_obj(&(pair->tail)))))
			return res;
	}

	return EU_RESULT_OK;
}

/** Releases any other resources associated to this pair that is not
 * automatically collected.
 * 
 * @param gc the garbage collector structure.
 * @param pair the pair to "destroy".
 * @return the result of the destruction.
 */
eu_result eupair_destroy(eu_gc* gc, eu_pair* pair) {
	return EU_RESULT_OK;
}

/* the language API */

/**
 * @addtogroup language_library
 * @{
 */

eu_value euapi_cell_car(europa* s, eu_cell* args) {
	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "car", 0));

	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head,
			EU_OBJTYPE_CELL));

	return car(ccar(args));
}

eu_value euapi_cell_cdr(europa* s, eu_cell* args) {
	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head,
			EU_OBJTYPE_CELL));
	
	return cdr(ccar(args));
}

eu_value euapi_cell_cons(europa* s, eu_cell* args) {
	eu_value v;
	v.type = EU_TYPE_OBJECT;
	v.value.object = eucell_make_pair(s, ccar(args), car(ccdr(args)));
	return v;
}

eu_value euapi_cell_is_pair(europa* s, eu_cell* args) {
	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "pair?", 0));

	return euval_from_boolean(euvalue_is_cell(ccar(args)));
}

eu_value euapi_cell_set_car(europa* s, eu_cell* args) {
	eu_value r;

	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-car!", 0));

	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head,
			EU_OBJTYPE_CELL));

	if (euvalue_is_null(ccdr(args)))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-cdr!", 1));

	eu_value2cell(ccar(args))->head = car(ccdr(args));

	r.type = EU_TYPE_NULL;
	return r;
}

eu_value euapi_cell_set_cdr(europa* s, eu_cell* args) {
	eu_value r;

	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-cdr!", 0));

	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head,
			EU_OBJTYPE_CELL));

	if (euvalue_is_null(ccdr(args)))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-cdr!", 1));

	eu_value2cell(ccar(args))->tail = car(ccdr(args));

	r.type = EU_TYPE_NULL;
	return r;
}

/**
 * @}
 */