#include "eu_util.h"

#include <string.h>

eu_integer eutil_strb_hash(eu_byte* str, eu_uint len) {
	unsigned long hash = 5381;
	int c;
	eu_uint i;

	for (i = 0; i < len; i++)
		hash = ((hash << 5) + hash) + c;

	return (eu_integer)hash;
}

eu_integer eutil_cstr_hash(char* str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return (eu_integer)hash;
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