/** Symbol implementation file.
 * 
 * @file eu_symbol.c
 * @author Leonardo G.
 */
#include "eu_symbol.h"

#include <string.h>

#include "eu_util.h"
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

	sym = (eu_symbol*)eugc_new_object(_eu_get_gc(s), EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
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
eu_integer eusymbol_hash(eu_symbol* sym) {
	if (sym == NULL)
		return 0;
	return _eusymbol_hash(sym);
}