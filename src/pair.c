/**
 * @file pair.c
 * @brief Pair (cons cell) related operations.
 * @author Leonardo G.
 */
#include "eu_pair.h"

#include "eu_gc.h"
#include "eu_error.h"
#include "eu_ccont.h"
#include "eu_rt.h"
#include "eu_number.h"

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
	eu_pair* pair;

	pair = cast(eu_pair*,eugc_new_object(s, EU_TYPE_PAIR |
		EU_TYPEFLAG_COLLECTABLE, sizeof(eu_pair)));
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
eu_result eupair_mark(europa* s, eu_gcmark mark, eu_pair* pair) {
	eu_result res;

	if (s == NULL || pair == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* try and mark head */
	if (_euvalue_is_collectable(&(pair->head))) {
		if ((res = mark(s, _euvalue_to_obj(&(pair->head)))))
			return res;
	}

	/* try and mark tail */
	if (_euvalue_is_collectable(&(pair->tail))) {
		if ((res = mark(s, _euvalue_to_obj(&(pair->tail)))))
			return res;
	}

	return EU_RESULT_OK;
}

/** Gets a hash code for a pair object.
 * 
 * @param s the europa state.
 * @param pair the target pair.
 * @return the hash of the pair.
 */
eu_uinteger eupair_hash(eu_pair* pair) {
	/* TODO: change behavior when using a moving gc */
	return cast(eu_integer, pair);
}

eu_value* eulist_tail(europa* s, eu_pair* list, int k) {
	eu_value *v, lv;
	int i;

	_eu_makepair(&lv, list);

	for (i = k, v = &lv; i > 0 && _euvalue_is_type(v, EU_TYPE_PAIR);
		v = _eupair_tail(_euvalue_to_pair(v)), i--) {
	}

	if (!_euvalue_is_type(v, EU_TYPE_PAIR)) {
		eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"list-ref %d on improper or short list.", k);
		return NULL;
	}

	return v;
}

eu_value* eulist_ref(europa* s, eu_pair* list, int k) {
	eu_value* v;

	v = eulist_tail(s, list, k);
	if (v == NULL)
		return NULL;

	return _eupair_head(_euvalue_to_pair(v));
}

int eulist_length(europa* s, eu_pair* list) {
	eu_value *v, lv;
	int length;

	_eu_makepair(&lv, list);

	length = 0;
	while (_euvalue_is_pair(v)) {
		v = _eupair_tail(_euvalue_to_pair(v));
		length++;
	}

	if (!_euvalue_is_null(v)) {
		return -length;
	}

	return length;
}

/* the language API */

/**
 * @addtogroup language_library
 * @{
 */

/**
 * @brief Registers pair procedures in the global environment.
 * 
 * @param s The Europa state.
 * @return The result of the operation.
 */
eu_result euapi_register_pair(europa* s) {
	eu_table* env;

	env = s->env;

	/* */
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "pair?", euapi_pairQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "cons", euapi_cons));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "car", euapi_car));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "cdr", euapi_cdr));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "set-car!", euapi_set_carB));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "set-cdr!", euapi_set_cdrB));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "null?", euapi_nullQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "list", euapi_list));

	return EU_RESULT_OK;
}

eu_result euapi_pairQ(europa* s) {
	eu_value* obj;

	/* check procedure arity */
	_eucc_arity_proper(s, 1);
	/* get first argument */
	_eucc_argument(s, obj, 0);
	/* set the return to a boolean that says whether the passed value is a pair */
	_eu_makebool(_eucc_return(s), _euvalue_is_pair(obj));

	return EU_RESULT_OK;
}

eu_result euapi_cons(europa* s) {
	eu_value *a, *b;
	eu_pair* p;

	/* check arity */
	_eucc_arity_proper(s, 2);
	/* read arguments */
	_eucc_argument(s, a, 0);
	_eucc_argument(s, b, 1);

	p = eupair_new(s, a, b);
	if (p == NULL) {
		return EU_RESULT_BAD_ALLOC;
	}

	_eu_makepair(_eucc_return(s), p);

	return EU_RESULT_OK;
}

eu_result euapi_car(europa* s) {
	eu_value *list;
	eu_pair* p;

	/* check arity */
	_eucc_arity_proper(s, 2);
	/* read arguments */
	_eucc_argument_type(s, list, 0, EU_TYPE_PAIR);
	/* set return to head */
	*_eucc_return(s) = *_eupair_head(_euvalue_to_pair(list));

	return EU_RESULT_OK;
}

eu_result euapi_cdr(europa* s) {
	eu_value *list;
	eu_pair* p;

	/* check arity */
	_eucc_arity_proper(s, 2);
	/* read arguments */
	_eucc_argument_type(s, list, 0, EU_TYPE_PAIR);
	/* set return to tail */
	*_eucc_return(s) = *_eupair_tail(_euvalue_to_pair(list));

	return EU_RESULT_OK;
}

eu_result euapi_set_carB(europa* s) {
	eu_value *pair, *value;
	eu_pair* p;

	/* check arity */
	_eucc_arity_proper(s, 2);
	/* read arguments */
	_eucc_argument_type(s, pair, 0, EU_TYPE_PAIR);
	_eucc_argument_type(s, value, 1, EU_TYPE_PAIR);

	*_eupair_head(_euvalue_to_pair(pair)) = *value;
	*_eucc_return(s) = _null;

	return EU_RESULT_OK;
}

eu_result euapi_set_cdrB(europa* s) {
	eu_value *pair, *value;
	eu_pair* p;

	/* check arity */
	_eucc_arity_proper(s, 2);
	/* read arguments */
	_eucc_argument_type(s, pair, 0, EU_TYPE_PAIR);
	_eucc_argument(s, value, 1);

	*_eupair_tail(_euvalue_to_pair(pair)) = *value;
	*_eucc_return(s) = _null;

	return EU_RESULT_OK;
}

eu_result euapi_nullQ(europa* s) {
	eu_value* obj;

	/* check procedure arity */
	_eucc_arity_proper(s, 1);
	/* get first argument */
	_eucc_argument(s, obj, 0);
	/* set the return to a boolean that says whether the passed value is a pair */
	_eu_makebool(_eucc_return(s), _euvalue_is_null(obj));

	return EU_RESULT_OK;
}

eu_result euapi_list(europa* s) {
	*_eucc_return(s) = s->rib;
	return EU_RESULT_OK;
}

/**
 * @}
 */
