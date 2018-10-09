/** Symbol implementation file.
 * 
 * @file symbol.c
 * @author Leonardo G.
 */
#include "eu_symbol.h"

#include <string.h>

#include "eu_util.h"
#include "eu_number.h"
#include "utf8.h"

/*
 * Symbols:
 * 
 * The symbol internal structure is effectively
 * struct symbol {
 *   // common header
 *   gcobj* _next;
 *   byte _type;
 *   byte _mark;
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
eu_symbol* eusymbol_new(europa* s, void* text) {
	eu_symbol* sym;

	/* if symbol text is not a valid utf-8 string, the call is invalid. */
	if (utf8valid(text))
		return NULL;

	size_t text_size = utf8size(text);

	sym = (eu_symbol*)eugc_new_object(s, EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_symbol) + text_size);
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
void* eusymbol_text(eu_symbol* sym) {
	if (sym == NULL)
		return NULL;
	return _eusymbol_text(sym);
}

/** Returns the object's hash.
 * 
 * @param sym The symbol object.
 * @return The obejct's hash.
 */
eu_uinteger eusymbol_hash(eu_symbol* sym) {
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
eu_result eusymbol_eqv(eu_value* a, eu_value* b, eu_value* out) {
	eu_symbol *sa, *sb;
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
eu_bool eusymbol_equal_cstr(eu_value* vsym, const char* cstr) {
	if (!vsym || !cstr)
		return EU_FALSE;
	return !utf8cmp(_eusymbol_text(_euvalue_to_symbol(vsym)), cstr);
}