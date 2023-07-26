/** Symbol implementation file.
 *
 * @file symbol.c
 * @author Leonardo G.
 */
#include "europa/symbol.h"

#include <string.h>

#include "europa/util.h"
#include "europa/number.h"
#include "utf8.h"
#include "europa/ccont.h"

/*
 * Symbols:
 *
 * The symbol internal structure is effectively
 * struct symbol {
 *   // common header
 *   gcobj* _next;
 *   byte _type;
 *   byte _color;
 *
 *   // structure data
 *   integer hash;
 *   byte _text;
 * }
 *
 * symbols are immutable, so we can keep its text in the symbol structure space
 * itself (and contribute less to memory fragmentation). the _text byte serves
 * as the address of the text's start.
 */

/** Creates a new symbol with a given text.
 *
 * @param s the Europa state.
 * @param text the symbol's text.
 * @return The created symbol.
 */
struct europa_symbol* eusymbol_new(europa* s, void* text) {
	struct europa_symbol* sym;
	struct europa_value *tv;

	/* if symbol text is not a valid utf-8 string, the call is invalid. */
	if (utf8valid(text))
		return NULL;

	/* check if symbol already exists in internalized table */
	if (_eu_global(s)->internalized) {
		if (eutable_rget_symbol(s, _eu_global(s)->internalized, text, &tv))
			return NULL;
		if (tv != NULL) {
			/* returning it if it does */
			return _euvalue_to_symbol(tv);
		} /* TODO: eventually add symbols to the internalized table dynamically */
	}

	size_t text_size = utf8size(text);

	sym = (struct europa_symbol*)eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(struct europa_symbol) + text_size);
	if (sym == NULL)
		return NULL;

	/* copy the text */
	memcpy(&(sym->_text), text, text_size);
	/* calculate hash */
	sym->hash = eutil_cstr_hash(_eusymbol_text(sym));

	return sym;
}

/** Returns the address of the utf-8 text buffer.
 *
 * @param sym The symbol structure.
 * @return The managed utf-8 string.
 *
 * @remarks The buffer will be managed by the GC and it will not outlive the
 * object, so if you need a lasting copy of it, make it yourself.
 */
void* eusymbol_text(struct europa_symbol* sym) {
	if (sym == NULL)
		return NULL;
	return _eusymbol_text(sym);
}

/** Returns the object's hash.
 *
 * @param sym The symbol object.
 * @return The obejct's hash.
 */
eu_uinteger eusymbol_hash(struct europa_symbol* sym) {
	if (sym == NULL)
		return 0;
	return _eusymbol_hash(sym);
}

/** Hashes a string as if it were a symbol.
 *
 * @param str The string representing the symbol.
 * @return The resulting hash.
 */
eu_integer eusymbol_hash_cstr(const char* str){
	if (str == NULL)
		return 0;
	return eutil_cstr_hash(str);
}

/** Checks whether two symbols are `eqv?`.
 *
 * @param a The first symbol.
 * @param b The other symbol.
 * @param out Where to place the boolean result.
 * @returns The result of executing the operation.
 */
int eusymbol_eqv(struct europa_value* a, struct europa_value* b, struct europa_value* out) {
	struct europa_symbol *sa, *sb;
	int v;

	sa = _euvalue_to_symbol(a);
	sb = _euvalue_to_symbol(b);

	/* if hashes don't match, they are definitely different */
	if (_eusymbol_hash(sa) != _eusymbol_hash(sb)) {
		v = EU_FALSE;
		goto end;
	}

	/* hashes match, check if texts are the same */
	v = !utf8cmp(_eusymbol_text(sa), _eusymbol_text(sb));

	end:
	_eu_makebool(out, v);
	return EU_RESULT_OK;
}

/** Checks whether a symbol's text matches a given C string.
 *
 * @param vsym The value symbol.
 * @param cstr The C string.
 * @return Whether the two match.
 */
eu_bool eusymbol_equal_cstr(struct europa_value* vsym, const char* cstr) {
	if (!vsym || !cstr)
		return EU_FALSE;
	return !utf8cmp(_eusymbol_text(_euvalue_to_symbol(vsym)), cstr);
}

/**
 * @addtogroup language_library
 * @{
 */

int euapi_register_symbol(europa* s) {
	struct europa_table* env;

	env = s->env;

	_eu_checkreturn(eucc_define_cclosure(s, env, env, "symbol?", euapi_symbolQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "symbol->string", euapi_symbol_to_string));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "string->symbol", euapi_string_to_symbol));

	return EU_RESULT_OK;
}

int euapi_symbolQ(europa* s) {
	struct europa_value* object;

	_eucc_arity_proper(s, 1); /* check arity */
	_eucc_argument(s, object, 0); /* get argument */

	_eu_makebool(_eucc_return(s), _euvalue_is_type(object, EU_TYPE_SYMBOL));
	return EU_RESULT_OK;
}

int euapi_symbolEQ(europa* s) {
	struct europa_value *current, *previous, *cv, *pv;

	_eucc_arity_improper(s, 2); /* check arity */

	/* initialize previous and current arguments */
	previous = _eucc_arguments(s);
	current = _eupair_tail(_euvalue_to_pair(previous));

	while (!_euvalue_is_null(current)) {
		/* get arguments */
		pv = _eupair_head(_euvalue_to_pair(previous));
		cv = _eupair_head(_euvalue_to_pair(current));

		/* check whether symbols are equal */
		_eu_checkreturn(eusymbol_eqv(pv, cv, _eucc_return(s)));
		if (_eucc_return(s)->value.boolean == EU_FALSE)
			return EU_RESULT_OK;

		/* advance argument */
		previous = current;
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	return EU_RESULT_OK;
}

int euapi_symbol_to_string(europa* s) {
	struct europa_value* symbol;
	eu_string* str;

	_eucc_arity_proper(s, 1); /* check arity */
	_eucc_argument_type(s, symbol, 0, EU_TYPE_SYMBOL); /* get argument */

	/* create new string with symbol text */
	str = eustring_new(s, _eusymbol_text(_euvalue_to_symbol(symbol)));
	if (str == NULL)
		return EU_RESULT_BAD_ALLOC;

	_eu_makestring(_eucc_return(s), str);
	return EU_RESULT_OK;
}

int euapi_string_to_symbol(europa* s) {
	struct europa_value* string;
	struct europa_symbol* sym;

	_eucc_arity_proper(s, 1); /* check arity */
	_eucc_argument_type(s, string, 0, EU_TYPE_STRING); /* get argument */

	/* create new symbol with string text */
	sym = eusymbol_new(s, _eustring_text(_euvalue_to_string(string)));
	if (sym == NULL)
		return EU_RESULT_BAD_ALLOC;

	_eu_makesym(_eucc_return(s), sym);
	return EU_RESULT_OK;
}



/**
 * @}
 */
