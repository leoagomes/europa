/** Closure structure routines.
 * 
 * @file closure.c
 * @author Leonardo G.
 */
#include "eu_closure.h"

/** Instantiates a new closure structure.
 * 
 * @param s The Europa state.
 * @param cfunc The C function to create a closure around.
 * @param body The closure's body.
 * @param formals The function's formals.
 * @param env Its declaration environment.
 * @return The created closure.
 */
eu_closure* eucl_new(europa* s, eu_cfunc cfunc, eu_pair* body, eu_pair* formals, eu_table* env) {
	eu_closure* cl;

	cl = eugc_new_object(s, EU_TYPE_CLOSURE | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_closure));
	if (cl == NULL)
		return NULL;

	cl->body = body;
	cl->formals = formals;
	cl->env = env;

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

	/* mark body */
	if (cl->body) {
		_eu_checkreturn(eupair_mark(s, mark, cl->body));
	}

	/* mark formals */
	if (cl->formals) {
		_eu_checkreturn(eupair_mark(s, mark, cl->formals));
	}

	/* mark environment table */
	if (cl->env) {
		_eu_checkreturn(eutable_mark(s, mark, cl->formals));
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
