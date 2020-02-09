/** Core state and environment structure routines.
 *
 * @file europa.c
 * @author Leonardo G.
 */
#include "europa/eu.h"

#include "europa/gc.h"
#include "europa/object.h"
#include "europa/table.h"
#include "europa/symbol.h"
#include "europa/number.h"
#include "europa/error.h"
#include "europa/rt.h"
#include "europa/port.h"
#include "europa/ports/memory.h"

#include <stdarg.h>
#include <stdio.h>

eu_result global_basic_init(eu_global* g, eu_realloc f, void* ud, eu_cfunc panic) {
	/* pretend this is a normal GC object */
	g->_previous = g->_next = cast(eu_object*, g);
	g->_mark = EUGC_COLOR_WHITE;
	g->_type = EU_TYPE_GLOBAL | EU_TYPEFLAG_COLLECTABLE;

	/* initialize the garbage collector */
	_eu_checkreturn(eugc_init(_euglobal_gc(g), ud, f));

	/* initialize other fields */
	g->panic = panic;
	g->internalized = NULL;

	return EU_RESULT_OK;
}

eu_result global_environment_init(europa* s, int pred_size) {
	s->global->env = eutable_new(s, pred_size);
	if (s->global->env == NULL)
		return EU_RESULT_BAD_ALLOC;

	return EU_RESULT_OK;
}

#define INTERNALIZED_COUNT 256

#define __add_symbol(sname, sym, s, t, sv, tv) do {\
	sym = eusymbol_new(s, sname);\
	if (sym == NULL)\
		return EU_RESULT_BAD_ALLOC;\
	_eu_makesym(&sv, sym);\
	_eu_checkreturn(eutable_create_key(s, t, &sv, &tv));\
	if (tv == NULL)\
		return EU_RESULT_BAD_RESOURCE;\
	*tv = sv;\
} while (0)

/** Bootstraps the internalized table.
 *
 * @param s The Europa state.
 * @return The operation's result.
 */
eu_result euglobal_bootstrap_internalized(europa* s) {
	eu_table* t;
	eu_symbol* sym;
	eu_value symvalue, *tvalue;

	/* create the table */
	t = eutable_new(s, INTERNALIZED_COUNT);
	if (t == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* todo: put useful values here */
	__add_symbol("quote", sym, s, t, symvalue, tvalue);
	__add_symbol("unquote", sym, s, t, symvalue, tvalue);
	__add_symbol("quasiquote", sym, s, t, symvalue, tvalue);
	__add_symbol("eq?", sym, s, t, symvalue, tvalue);
	__add_symbol("eqv?", sym, s, t, symvalue, tvalue);
	__add_symbol("equal?", sym, s, t, symvalue, tvalue);
	__add_symbol("@@args", sym, s, t, symvalue, tvalue);
	__add_symbol("@@call", sym, s, t, symvalue, tvalue);

	/* set internalized*/
	_eu_global(s)->internalized = t;

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
europa* eu_new(eu_realloc f, void* ud, eu_cfunc panic, eu_result* err) {
	europa* s;
	eu_global* gl;
	eu_result res;

	/* Even the main state is just an instance in the global's GC, so we must
	 * first allocate a global. */
	gl = (f)(ud, NULL, sizeof(eu_global));
	if (gl == NULL) {
		/* set error variable */
		_checkset(err, EU_RESULT_BAD_ALLOC);
		return NULL;
	}

	/* now we initialize the global state at least to a point where the GC works
	 * properly */
	if ((res = global_basic_init(gl, f, ud, panic))) {
		_checkset(err, EU_RESULT_ERROR);
		/* free the allocated global */
		(f)(ud, gl, 0);
		return NULL;
	}

	/* grab the state as a gc object */
	europa fake;
	fake.global = gl;
	s = cast(europa*, eugc_new_object(&fake, EU_TYPE_STATE | EU_TYPEFLAG_COLLECTABLE,
		sizeof(europa)));
	if (s == NULL) {
		s = &fake;
		goto fail;
	}

	s->input_port = NULL;
	s->output_port = NULL;
	s->error_port = NULL;

	s->err = NULL;
	s->global = gl;
	s->global->main = s;

	/* insert the global into the GC's root set */
	if ((res = eugc_move_to_root(s, cast(eu_object*, gl)))) {
		_checkset(err, res);
		goto fail;
	}
	if ((res = eugc_move_to_root(s, cast(eu_object*, s)))) {
		_checkset(err, res);
		goto fail;
	}

	/* just create the environment table */
	if ((res = global_environment_init(s, 0))) {
		_checkset(err, res);
		goto fail;
	}

	/* bootstrap internalized table */
	if ((res = euglobal_bootstrap_internalized(s))) {
		_checkset(err, res);
		goto fail;
	}

	/* setup initial state */
	if ((res = euvm_initialize_state(s))) {
		_checkset(err, res);
		goto fail;
	}

	return s;

	fail:
	/* terminate the gc */
	if ((res = eugc_destroy(s))) {
		_checkset(err, res);
	}
	(f)(ud, gl, 0);
	return NULL;
}

/**
 * @brief Sets the state error to an error with given flags, nested errors and
 * message.
 *
 * @param s The Europa state.
 * @param flags The error flags.
 * @param nested Any nested error objects.
 * @param text The message.
 * @return The result of the operation.
 */
eu_result eu_set_error(europa* s, int flags, eu_error* nested, void* text) {
	if (!s || !text)
		return EU_RESULT_NULL_ARGUMENT;

	/* create error with message, set to s->err */
	_eu_err(s) = euerror_new(s, flags, text, nested);
	if (_eu_err(s) == NULL)
		return EU_RESULT_BAD_ALLOC;

	return EU_RESULT_OK;
}

/**
 * @brief Sets the state error to an error object with given flags, nested errors
 * and message in the printf format.
 *
 * @param s The Europa state.
 * @param flags The error flags.
 * @param nested Any nested error objects.
 * @param len The maximum length for the message.
 * @param fmt The message's format.
 * @param ... Any printf-like arguments.
 * @return The result of the operation.
 */
eu_result eu_set_error_nf(europa* s, int flags, eu_error* nested, size_t len,
	const char* fmt, ...) {
	char buf[len];

	/* call snprintf with given arguments */
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, len, fmt, args);
	va_end(args);

	/* create the error object */
	_eu_err(s) = euerror_new(s, flags, buf, nested);
	if (_eu_err(s) == NULL)
		return EU_RESULT_BAD_ALLOC;

	return EU_RESULT_OK;
}

/**
 * @brief Runs a string.
 *
 * @param s The Europa state.
 * @param text The string to execute.
 * @param out Where to place the result.
 * @return The result of the operation.
 */
eu_result eu_do_string(europa* s, void* text, eu_value* out) {
	eu_port* p;
	eu_value obj;

	/* create a memory port for the text */
	p = _eumport_to_port(eumport_from_str(s, EU_PORT_FLAG_INPUT | EU_PORT_FLAG_TEXTUAL, text));
	if (p == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* read an object from the port */
	_eu_checkreturn(euport_read(s, p, &obj));

	/* evaluate it */
	_eu_checkreturn(eurt_evaluate(s, &obj, out));

	return EU_RESULT_OK;
}

/**
 * @brief Terminates a state.
 *
 * @param s The target state.
 * @return The result of the operation.
 */
eu_result eu_terminate(europa* s) {
	eu_result res;
	eu_global* gl;
	eu_realloc f;
	void* ud;

	if (s != s->global->main) {
		/* if we're not the main state, leave this job to the GC */
		return EU_RESULT_OK;
	}

	/* save the free function, its user data and the global state */
	gl = s->global;
	f = gl->gc.realloc;
	ud = gl->gc.ud;

	/* remove current state and global from gc */
	_eu_checkreturn(eugc_remove_object(s, cast(eu_object*, gl)));
	_eu_checkreturn(eugc_remove_object(s, cast(eu_object*, s)));

	/* destroy everything in the GC */
	res = eugc_destroy(s);

	/* release state and global */
	(f)(ud, s, 0);
	(f)(ud, gl, 0);

	return res; /* return whether the GC destruction was OK */
}

/**
 * @brief Returns the hash of an Europa state.
 *
 * @param s The target state.
 * @return The hash.
 */
eu_uinteger eustate_hash(europa* s) {
	return cast(eu_integer, s);
}

/**
 * @brief Returns the hash of a global state.
 *
 * @param gl The target global.
 * @return The hash.
 */
eu_uinteger euglobal_hash(eu_global* gl) {
	return cast(eu_integer, gl);
}

/**
 * @brief Marks references of an Europa State.
 *
 * @param s The Europa state.
 * @param mark The marking function.
 * @param state The target state (to be marked).
 * @return The result of the operation.
 */
eu_result eustate_mark(europa* s, eu_gcmark mark, europa* state) {
	if (!s || !mark || !state)
		return EU_RESULT_NULL_ARGUMENT;

	if (state->ccl) { /* state is executing something */
		/* mark current closure */
		_eu_checkreturn(mark(s, _euclosure_to_obj(state->ccl)));

		/* mark its environment */
		_eu_checkreturn(mark(s, _eutable_to_obj(state->env)));

		/* mark the stack */
		_eu_checkreturn(mark(s, _eucont_to_obj(state->previous)));

		/* mark the rib */
		if (_euvalue_is_collectable(&state->rib)) {
			_eu_checkreturn(mark(s, state->rib.value.object));
		}

		/* mark the accumulator */
		if (_euvalue_is_collectable(&state->acc)) {
			_eu_checkreturn(mark(s, state->acc.value.object));
		}
	}

	/* mark the global state (which should be in the root set, anyway) */
	if (state->global) {
		_eu_checkreturn(mark(s, cast(eu_object*, state->global)));
	}

	/* mark any errors */
	if (state->err) {
		_eu_checkreturn(mark(s, _euerror_to_obj(state->err)));
	}

	return EU_RESULT_OK;
}

/**
 * @brief Marks references of a global Europa State.
 *
 * @param s The Europa state.
 * @param mark The marking function.
 * @param gl The target global state (to be marked).
 * @return The result of the operation.
 */
eu_result euglobal_mark(europa* s, eu_gcmark mark, eu_global* gl) {
	if (!s || !mark || !gl)
		return EU_RESULT_NULL_ARGUMENT;

	/* mark the main state */
	_eu_checkreturn(mark(s, cast(eu_object*, gl->main)));

	/* mark internalized table */
	_eu_checkreturn(mark(s, _eutable_to_obj(gl->internalized)));

	/* mark global environment */
	_eu_checkreturn(mark(s, _eutable_to_obj(gl->env)));

	return EU_RESULT_OK;
}

/**
 * @brief Recovers the state from an error.
 *
 * @param s The Europa state.
 * @param[out] err Where to place the current error. Ignored if NULL.
 * @return The result of the operation.
 */
eu_result eu_recover(europa* s, eu_error** err) {
	if (err)
		*err = s->err;

	euvm_initialize_state(s);
	s->err = NULL;

	return EU_RESULT_OK;
}
