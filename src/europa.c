/** Core state and environment structure routines.
 * 
 * @file europa.c
 * @author Leonardo G.
 */
#include "europa.h"

#include "eu_table.h"
#include "eu_symbol.h"
#include "eu_number.h"
#include "eu_error.h"
#include "eu_rt.h"
#include "eu_port.h"
#include "port/eu_mport.h"

#include <stdarg.h>
#include <stdio.h>

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
	g->internalized = NULL;

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

	/* bootstrap internalized table */
	if ((res = euglobal_bootstrap_internalized(s))) {
		_checkset(err, res);
	}

	/* setup initial state */
	if ((res = euvm_initialize_state(s))) {
		_checkset(err, res);
	}

	return s;
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
	p = _eumport_to_port(eumport_from_str(s, EU_PORT_FLAG_OUTPUT | EU_PORT_FLAG_TEXTUAL, text);
	if (p == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* read an object from the port */
	_eu_checkreturn(euport_read(s, p, &obj));

	/* evaluate it */
	_eu_checkreturn(eu_evaluate(s, &obj, out));

	return EU_RESULT_OK;
}
