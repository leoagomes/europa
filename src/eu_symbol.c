/** Symbol implementation file.
 * 
 * @file eu_symbol.c
 * @author Leonardo G.
 */
#include "eu_symbol.h"

#include <string.h>

#include "eu_util.h"
#include "utf8.h"

/**
 * Symbols:
 * 
 * The symbol internal structure is effectively
 * struct symbol {
 *   // common header
 *   gcobj* next;
 *   byte type;
 *   byte mark;
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

	sym = (eu_symbol*)eugc_new_object(eu_get_gc(s), EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_symbol) + text_size);
	if (sym == NULL)
		return NULL;

	/* copy the text */
	memcpy(&(sym->_text), text, text_size);
	/* calculate hash */
	sym->hash = eutil_cstr_hash(&(sym->_text));

	return sym;
}