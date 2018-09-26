/** Core state and environment structure routines.
 * 
 * @file europa.c
 * @author Leonardo G.
 */
#include "europa.h"

/** Initializes a global environment.
 * 
 * @param g the global paremeter.
 * @param f the realloc-like function.
 * @param ud user data for the f function.
 * @return Whether any errors occured.
 */
eu_result euglobal_init(eu_global* g, eu_realloc f, void* ud, eu_cfunc panic) {
	/* initialize the garbage collector */
	_eu_checkreturn(eugc_init(_euglobal_gc(g), ud, f));

	/* initialize other fields */
	g->panic = panic;

	return EU_RESULT_OK;
}

#define _checkset(vptr, val) \
	if (vptr) \
		*(vptr) = (val)

/** Allocates and initializes a new europa state.
 * 
 * @param f The realloc-like function.
 * @param ud Userdata for the realloc function.
 * @param err Where to place an error code if any errors happen. Ignored if
 * NULL.
 * @return A new main europa state, with a new global state.
 */
europa* europa_new(eu_realloc f, void* ud, eu_cfunc panic, eu_result* err) {
	europa* s;
	eu_result res;

	/* allocate memory for the state */
	s = (f)(ud, NULL, sizeof(europa));
	if (s == NULL) { /* allocation failed */
		/* set error variable */
		_checkset(err, EU_RESULT_BAD_ALLOC);
		return NULL;
	}

	/* allocate memory for the global */
	_eu_global(s) = (f)(ud, NULL, sizeof(eu_global));
	if (_eu_global(s) == NULL) { /* allocation failed */
		/* free the state we just allocated */
		(f)(ud, s, 0);
		/* set error variable */
		_checkset(err, EU_RESULT_BAD_ALLOC);
		return NULL;
	}

	/* try initializing the global environment */
	if ((res = euglobal_init(_eu_global(s), f, ud, panic))) {
		/* free all allocated resources */
		(f)(ud, _eu_global(s), 0);
		(f)(ud, s, 0);

		/* set the error variable if it exists */
		_checkset(err, res);
		return NULL;
	}

	/* set the current context as the main for the global environment */
	_eu_global(s)->main = s;

	return s;
}