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

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return (eu_integer)hash;
}