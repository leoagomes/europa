#include "eu_string.h"

#include <string.h>

#include "eu_util.h"
#include "utf8.h"

/* Strings, like symbols, hold their text along their structure's memory.
 * 
 * Memory for a eu_string object is allocated along with the memory needed to
 * keep a copy of their text. This makes it difficult to change a string's
 * length, so we make all strings immutable.
 */

/** Creates a new string object, managed by the GC.
 * 
 * @param s The Europa state.
 * @param text The UTF-8 text for the string.
 * @return The resulting string object.
 */
eu_string* eustring_new(europa* s, void* text) {
	eu_string* str;

	if (s == NULL || text == NULL || utf8valid(text))
		return NULL;

	/* allocate enough memory for text data and structure data */
	size_t text_size = utf8size(text);
	str = _euobj_to_string(eugc_new_object(_eu_get_gc(s), EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_string) + text_size));
	if (str == NULL)
		return NULL;

	str->size = text_size;

	/* copy the text into the structure */
	memcpy(_eustring_text(str), text, text_size);
	/* calculate its hash */
	str->hash = eutil_cstr_hash(_eustring_text(str));

	return str;
}

eu_string* eustring_withsize(europa* s, size_t textsize) {
	eu_string* str;

	if (s == NULL || textsize <= 0)
		return NULL;

	str = _euobj_to_string(eugc_new_object(_eu_get_gc(s),
		EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE, sizeof(eu_string) + textsize));
	if (str == NULL)
		return NULL;

	str->size = textsize;

	return str;
}

/** Returns the UTF-8 text buffer of the string object.
 * 
 * @param str The string object.
 * @return The UTF-8 string buffer.
 * 
 * @remarks The buffer is managed by the GC and will be realeased along with the
 * string object. If the object is released, the buffer is released, so if you
 * need the buffer to potentially outlive the object, make a copy of it.
 */
void* eustring_text(eu_string* str) {
	if (str == NULL)
		return NULL;
	return _eustring_text(str);
}

/** Gets the hash for the string object.
 * 
 * @param str The string object.
 * @return The object's hash.
 */
eu_integer eustring_hash(eu_string* str) {
	if (str == NULL)
		return 0;
	return _eustring_hash(str);
}

/** Calculates a string's hash and sets its hash field to it.
 * 
 * @param str The target string.
 * @return The recalculated hash.
 */
eu_integer eustring_rehash(eu_string* str) {
	if (str == NULL)
		return 0;
	str->hash = eutil_cstr_hash(_eustring_text(str));
	return str->hash;
}
