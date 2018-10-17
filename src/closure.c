/** Closure structure routines.
 * 
 * @file closure.c
 * @author Leonardo G.
 */
#include "eu_rt.h"

/** Instantiates a new closure structure.
 * 
 * @param s The Europa state.
 * @param cfunc The C function to create a closure around.
 * @return The created closure.
 */
eu_closure* eucl_new(europa* s, eu_cfunc cfunc, void* body, eu_pair* formals, eu_table* env) {
	eu_closure* cl;

	// TODO: reimplement

	return cl;
}

/** Marks a closure.
 * 
 * @param s The Europa state.
 * @param mark The marking function.
 * @param cl The target closure.
 * @return The result of the operation.
 */
eu_result eucl_mark(europa* s, eu_gcmark mark, eu_closure* cl) {
	if (!s || !mark || !cl)
		return EU_RESULT_NULL_ARGUMENT;

	if (cl->proto) {
		_eu_checkreturn(mark(s, _euproto_to_obj(cl->proto)));
	}

	if (cl->env) {
		_eu_checkreturn(mark(s, _eutable_to_obj(cl->env)));
	}

	return EU_RESULT_OK;
}


/** Returns a closure's hash.
 * 
 * @param cl The target closure.
 * @return Its hash.
 */
eu_integer eucl_hash(eu_closure* cl) {
	return cast(eu_integer, cl);
}

/** Destroys a closure.
 * 
 * @param s The Europa state.
 * @param cl The target closure.
 * @return The result of the operation.
 */
eu_result eucl_destroy(europa* s, eu_closure* cl) {
	return EU_RESULT_OK;
}
