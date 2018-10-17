/** Error type implementation.
 * 
 * @file error.c
 * @author Leonardo G.
 */
#include "eu_error.h"

#include <string.h>

#include "utf8.h"

eu_error* euerror_new(europa* s, int flags, void* text, eu_error* nested) {
	eu_error* err;
	size_t text_size;

	if (!s || !text)
		return NULL;

	text_size = utf8size(text);

	err = _euobj_to_error(eugc_new_object(s, EU_TYPEFLAG_COLLECTABLE | EU_TYPE_ERROR,
		sizeof(eu_error) + text_size));
	if (err == NULL)
		return NULL;

	err->flags = flags;
	memcpy(&(err->_msg), text, text_size);

	err->nested = nested;

	return err;
}

void* euerror_message(eu_error* err) {
	return _euerror_message(err);
}

eu_uinteger euerror_hash(eu_error* err) {
	return (eu_integer)err;
}
