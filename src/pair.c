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
eu_integer eupair_hash(eu_pair* pair) {
	/* TODO: change behavior when using a moving gc */
	return cast(eu_integer, pair);
}

/* the language API */

/**
 * @addtogroup language_library
 * @{
 */

/**
 * @}
 */
