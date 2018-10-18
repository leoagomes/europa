/** Utility functions.
 * 
 * @file util.c
 * @author Leonardo G.
 */
#include "eu_util.h"

#include "eu_pair.h"

#include <string.h>

eu_uinteger eutil_strb_hash(eu_byte* str, eu_uint len) {
	eu_uinteger hash = 5381;
	int c;
	eu_uint i;

	for (i = 0; i < len; i++)
		hash = ((hash << 5) + hash) + c;

	return (eu_integer)hash;
}

eu_uinteger eutil_cstr_hash(const char* str) {
	eu_uinteger hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return (eu_uinteger)hash;
}

int unicodetoutf8(int c) {
	int utf = 0;

	if (c < 0)
		return c;

	/* single byte chararcter */
	if (c < 0x80) {
		return c;
	}

	/* two byte characters */
	if (c < 0x800) {
		return ((0xC0 | (0x1F & (c >> 6))) << 8) |
			(0x80 | (0x3F & c));
	}

	/* three byte */
	if (c < 0x10000) {
		return ((0xE0 | (0xF & (c >> 12))) << 16) |
			((0x80 | (0x3F & (c >> 6))) << 8) |
			(0x80 | (0x3F & c));
	}

	/* four bytes */
	return ((0xF0 | (0x7 & (c >> 18))) << 24) |
		((0x80 | (0x3F & (c >> 12))) << 16) |
		((0x80 | (0x3F & (c >> 6))) << 8) |
		(0x80 | (0x3F & c));
}

/* this function's return value does not take into account the improper end of
 * the list:
 * list_length(s, (a b c)) = 3
 * list_length(s, (a b . c)) = 2
 * 
 * the number of elements may be considered (return + improper)
 * */
int eulist_length(europa* s, eu_value* v, int* improper) {
	int count;

	/* empty list: length = 0 */
	if (_euvalue_is_null(v))
		return 0;

	/* not even a list */
	if (!_euvalue_is_type(v, EU_TYPE_PAIR))
		return -1;

	/* count number of elements */
	count = 0;
	*improper = 0;
	while (!_euvalue_is_null(v)) {
		count++;
		v = _eupair_tail(_euvalue_to_pair(v));

		/* check if improper list */
		if (!_euvalue_is_type(v, EU_TYPE_PAIR)) {
			*improper = 1; /* mark improper */
			break;
		}
	}
	return count;
}
