/** Closure structure routines.
 * 
 * @file closure.c
 * @author Leonardo G.
 */
#include "eu_rt.h"

#include "eu_util.h"

/**
 * @brief Creates a new closure.
 * 
 * If the closure is an Europa one, it also intializes the closure's environment,
 * creating slots for each formal parameter and setting its "up environment"
 * to the passed argument, the creation environment.
 * 
 * @param s The Europa state.
 * @param cf The C function to close. (Only if C function.)
 * @param proto The europa function prototype.
 * @param env The environment where the closure was created.
 * @return eu_closure* 
 */
eu_closure* eucl_new(europa* s, eu_cfunc cf, eu_proto* proto, eu_table* env) {
	eu_closure* cl;
	int length, improper, i;
	eu_value *tv, *cv;

	/* allocate the closure */
	cl = _euobj_to_closure(eugc_new_object(s, EU_TYPE_CLOSURE |
		EU_TYPEFLAG_COLLECTABLE, sizeof(eu_closure)));
	if (cl == NULL)
		return NULL;

	cl->cf = cf; /* set c function */
	cl->proto = proto; /* set the prototype */

	/* create new environment table */
	length = eutil_list_length(s, _euproto_formals(proto), &improper);
	cl->env = eutable_new(s, (length < 0) ? 1 : (length + improper));
	if (cl->env == NULL)
		return NULL;
	_eutable_set_index(cl->env, env);

	if (length < 0) { /* check for identifier formal */
		if (eutable_create_key(s, cl->env, _euproto_formals(proto), &tv))
			return NULL;
		/* set originally to the empty list */
		_eu_makenull(tv);
	} else { /* list formals */
		for (cv = _euproto_formals(cl->proto);
			!_euvalue_is_null(cv) &&
			_euvalue_is_type(cv, EU_TYPE_PAIR);
			cv = _eupair_tail(_euvalue_to_pair(cv))) {
			/* try inserting the key into the environment */
			if (eutable_create_key(s, cl->env, cv, &tv))
				return NULL;
			/* set originally to empty list */
			_eu_makenull(tv);
		}

		if (!_euvalue_is_null(cv)) { /* improper list */
			if (eutable_create_key(s, cl->env, cv, &tv))
				return NULL;
			_eu_makenull(tv);
		}
	}

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
