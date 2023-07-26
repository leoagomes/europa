/** Utility functions.
 *
 * @file util.c
 * @author Leonardo G.
 */
#include "europa/util.h"

#include "europa/pair.h"
#include "europa/number.h"
#include "europa/symbol.h"
#include "europa/string.h"
#include "europa/port.h"
#include "europa/ports/file.h"
#include "europa/rt.h"

#include <string.h>
#include <stdlib.h>

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
int eutil_list_length(europa* s, struct europa_value* v, int* improper) {
	int count;

	/* initialize some values */
	count = 0;
	*improper = 0;

	/* empty list: length = 0 */
	if (_euvalue_is_null(v))
		return 0;

	/* not even a list */
	if (!_euvalue_is_type(v, EU_TYPE_PAIR))
		return -1;

	/* count number of elements */
	while (!_euvalue_is_null(v)) {
		count++;
		v = _eupair_tail(_euvalue_to_pair(v));

		/* check if improper list */
		if (!_euvalue_is_type(v, EU_TYPE_PAIR) &&
			!_euvalue_is_null(v)) {
			*improper = 1; /* mark improper */
			break;
		}
	}
	return count;
}

/**
 * @brief A realloc-like function, based on stdlib's realloc.
 *
 * @param ud User data.
 * @param ptr The pointer to realloc.
 * @param size The target memory size.
 * @return void* The returned buffer address.
 */
void* eutil_stdlib_realloclike(void* ud, void* ptr, size_t size) {
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	return realloc(ptr, size);
}

/**
 * @brief Registers standard library procedures.
 *
 * @param s
 * @return int
 */
int eutil_register_standard_library(europa* s) {

	/* set the correct environment */
	s->env = _struct europa_global_env(s);

	/* numeric standard library */
	_eu_checkreturn(euapi_register_number(s));
	/* pair and list functions*/
	_eu_checkreturn(euapi_register_pair(s));
	/* symbol functions */
	_eu_checkreturn(euapi_register_symbol(s));
	/* control functions */
	_eu_checkreturn(euapi_register_controls(s));
	/* port functions */
	_eu_checkreturn(euapi_register_port(s));

	return EU_RESULT_OK;
}

int eutil_set_standard_ports(europa* s) {
	struct europa_file_port* port;

	/* input port */
	port = eufport_from_file(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_INPUT, stdin);
	if (!port)
		return EU_RESULT_BAD_ALLOC;
	s->input_port = _eufport_to_port(port);

	/* output port */
	port = eufport_from_file(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_OUTPUT, stdout);
	if (!port)
		return EU_RESULT_BAD_ALLOC;
	s->output_port = _eufport_to_port(port);

	/* error port */
	port = eufport_from_file(s, EU_PORT_FLAG_TEXTUAL | EU_PORT_FLAG_OUTPUT, stderr);
	if (!port)
		return EU_RESULT_BAD_ALLOC;
	s->error_port = _eufport_to_port(port);

	return EU_RESULT_OK;
}
