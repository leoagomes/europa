/**
 * @file write.c
 * @author Leonardo G.
 * @brief Port object writing procedures.
 */
#include <stdio.h>

#include "europa/object.h"
#include "europa/string.h"
#include "europa/europa.h"
#include "europa/port.h"
#include "europa/error.h"
#include "europa/number.h"
#include "europa/character.h"
#include "europa/util.h"
#include "europa/symbol.h"
#include "europa/pair.h"
#include "europa/bytevector.h"
#include "europa/vector.h"
#include "europa/table.h"

#include "utf8.h"

/* big TODO: implement datum labels and write procedures that are guaranteed to
 * terminate.
 */

#define NUMBUF_SIZE 128
int euport_write_integer(europa* s, struct europa_port* port, eu_integer v) {
	char b[NUMBUF_SIZE];
	int pos, negative;
	eu_integer val;

	pos = 0;
	val = v;
	negative = val < 0;
	val = negative ? -val : val;

	/* transform values into digits and push in the b-stack */
	do {
		b[pos++] = '0' + (val % 10);
		val /= 10;
	} while (val != 0);

	/* if the number is negative, write a '-' */
	if (negative) {
		_eu_checkreturn(euport_write_char(s, port, '-'));
	}

	/* pop numbers from b-stack and write them out */
	while (pos > 0) {
		_eu_checkreturn(euport_write_char(s, port, b[--pos]));
	}

	return EU_RESULT_OK;
}

int euport_write_hex_uint(europa* s, struct europa_port* port, eu_uinteger val) {
	char d[NUMBUF_SIZE];
	int pos, tv;

	pos = 0;

	/* transform values into digits and push in the digit stack */
	do {
		tv = val % 0x10;
		d[pos++] = tv > 0x9 ? 'A' + (tv - 0x9) : '0' + tv;
		val /= 0x10;
	} while (val != 0);

	/* pop numbers from digit stack and write them out */
	while (pos > 0) {
		_eu_checkreturn(euport_write_char(s, port, d[--pos]));
	}

	return EU_RESULT_OK;
}

int write_string_literal(europa* s, struct europa_port* port, const void* text) {
	void *next;
	int c;

	/* write opening " */
	_eu_checkreturn(euport_write_char(s, port, '"'));

	next = cast(void*, text);
	while (next != NULL) {
		next = utf8codepoint(next, &c);

		if (c == 0)
			break;

		switch (c) {
		/* handle characters that need escaping */
		case '\\':
		case '"':
			_eu_checkreturn(euport_write_char(s, port, '\\'));
			_eu_checkreturn(euport_write_char(s, port, c));
			break;
		case '\n':
			_eu_checkreturn(euport_write_string(s, port, "\\n"));
			break;

		/* for every other character, just print it */
		default:
			_eu_checkreturn(euport_write_char(s, port, c));
			break;
		}
	}

	/* write closing " */
	_eu_checkreturn(euport_write_char(s, port, '"'));

	return EU_RESULT_OK;
}

int utf8isascii(void* text) {
	int c;

	do {
		text = utf8codepoint(text, &c);

		if (c > 0xFF)
			return 0;
	} while (text != NULL && c != 0);

	return 1;
}

int write_symbol_literal(europa* s, struct europa_port* port, void* text) {
	int ascii;

	/* check whether all chars are ascii */
	ascii = utf8isascii(text);

	/* in case not all chars are ascii, write the opening vertical line | */
	if (!ascii) {
		_eu_checkreturn(euport_write_char(s, port, '|'));
	}

	/* write the symbol's text */
	_eu_checkreturn(euport_write_string(s, port, text));

	/* in case not all chars are ascii, write the closing vertical line | */
	if (!ascii) {
		_eu_checkreturn(euport_write_char(s, port, '|'));
	}

	return EU_RESULT_OK;
}

int write_bytevector(europa* s, struct europa_port* port, struct europa_bytevector* bv) {
	int i;

	/* write bytevector "prefix": #u8( */
	_eu_checkreturn(euport_write_string(s, port, "#u8("));
	for (i = 0; i < _eubvector_length(bv); i++)
	{
		if (i != 0)
		{
			/* add a space before the value if not first element */
			_eu_checkreturn(euport_write_char(s, port, ' '));
		}
		/* write the integer value at current bytevector index */
		_eu_checkreturn(euport_write_integer(s, port, _eubvector_ref(bv, i)));
	}
	/* writen ending ')' */
	_eu_checkreturn(euport_write_char(s, port, ')'));

	return EU_RESULT_OK;
}

int write_char_literal(europa* s, struct europa_port* port, int c) {

	/* write char prefix #\ */
	_eu_checkreturn(euport_write_string(s, port, "#\\"));

	switch (c) {
	case '\n':
		return euport_write_string(s, port, "newline");
	case ' ':
		return euport_write_string(s, port, "space");
	case '\0':
		return euport_write_string(s, port, "null");
	case '\t':
		return euport_write_string(s, port, "tab");
	case '\b':
		return euport_write_string(s, port, "backspace");
	default:
		return euport_write_char(s, port, c);
	}

	return EU_RESULT_OK;
}

int write_real(europa* s, struct europa_port* port, eu_real v) {
	char buf[NUMBUF_SIZE];

	/* TODO: implement this in a way that does not depend on stdio */
	snprintf(buf, sizeof(buf), "%lf", v);
	euport_write_string(s, port, buf);

	return EU_RESULT_OK;
}

int write_number(europa* s, struct europa_port* port, struct europa_value* num) {
	if (_eunum_is_exact(num))
		return euport_write_integer(s, port, _eunum_i(num));
	else
		return write_real(s, port, _eunum_r(num));

	return EU_RESULT_OK;
}

int write_pair_simple(europa* s, struct europa_port* port, struct europa_value* p) {
	struct europa_value* v;

	/* write opening parenthesis ( */
	_eu_checkreturn(euport_write_char(s, port, '('));

	/* write all 'car's */
	for (v = p; _euvalue_is_pair(v); v = _eupair_tail(_euvalue_to_pair(v))) {
		if (v != p) { /* add a space character */
			_eu_checkreturn(euport_write_char(s, port, ' '));
		}
		_eu_checkreturn(euport_write_simple(s, port, _eupair_head(_euvalue_to_pair(v))));
	}

	/* check if improper list */
	if (!_euvalue_is_null(v)) {
		_eu_checkreturn(euport_write_string(s, port, " . "));
		_eu_checkreturn(euport_write_simple(s, port, v));
	}

	/* write closing parenthesis ) */
	_eu_checkreturn(euport_write_char(s, port, ')'));

	return EU_RESULT_OK;
}

int write_vector_simple(europa* s, struct europa_port* port, struct europa_vector* vec) {
	int i;

	/* write #( prefix */
	_eu_checkreturn(euport_write_string(s, port, "#("));

	/* print vector elements */
	for (i = 0; i < _euvector_length(vec); i++) {
		if (i != 0) {
			_eu_checkreturn(euport_write_char(s, port, ' '));
		}
		_eu_checkreturn(euport_write_simple(s, port, _euvector_ref(vec, i)));
	}

	/* write closing parenthesis ) */
	_eu_checkreturn(euport_write_char(s, port, ')'));

	return EU_RESULT_OK;
}

/**
 * @brief Writes an object. This may not terminate if structure is circular.
 *
 * @param s The Europa state.
 * @param port The target port.
 * @param v The target value.
 * @return The result of the operation.
 */
int euport_write_simple(europa* s, struct europa_port* port, struct europa_value* v) {
	switch (_euvalue_type(v)) {
	case EU_TYPE_NULL:
		return euport_write_string(s, port, "()");
	case EU_TYPE_BOOLEAN:
		_eu_checkreturn(euport_write_char(s, port, '#'));
		return euport_write_char(s, port, _euvalue_to_bool(v) ? 't' : 'f');
	case EU_TYPE_STRING:
		return write_string_literal(s, port, _eustring_text(_euvalue_to_string(v)));
	case EU_TYPE_SYMBOL:
		return write_symbol_literal(s, port, _eusymbol_text(_euvalue_to_symbol(v)));
	case EU_TYPE_CHARACTER:
		return write_char_literal(s, port, _euvalue_to_char(v));
	case EU_TYPE_BYTEVECTOR:
		return write_bytevector(s, port, _euvalue_to_bvector(v));
	case EU_TYPE_NUMBER:
		return write_number(s, port, v);

	case EU_TYPE_PAIR:
		return write_pair_simple(s, port, v);

	case EU_TYPE_VECTOR:
		return write_vector_simple(s, port, _euvalue_to_vector(v));
		break;

	case EU_TYPE_CLOSURE:
		_eu_checkreturn(euport_write_string(s, port, "#<procedure 0x"));
		_eu_checkreturn(euport_write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	case EU_TYPE_CONTINUATION:
		_eu_checkreturn(euport_write_string(s, port, "#<continuation 0x"));
		_eu_checkreturn(euport_write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');
		break;

	case EU_TYPE_EOF:
		_eu_checkreturn(euport_write_string(s, port, "#<eof>"));
		break;

	case EU_TYPE_ERROR:
		_eu_checkreturn(euport_write_string(s, port, "#<error: "));
		_eu_checkreturn(euport_write_string(s, port, _euerror_message(_euvalue_to_error(v))));
		return euport_write_char(s, port, '>');

	case EU_TYPE_GLOBAL:
		_eu_checkreturn(euport_write_string(s, port, "#<global 0x"));
		_eu_checkreturn(euport_write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	case EU_TYPE_PORT:
		_eu_checkreturn(euport_write_string(s, port, "#<port>"));
		break;

	case EU_TYPE_PROTO:
		_eu_checkreturn(euport_write_string(s, port, "#<prototype 0x"));
		_eu_checkreturn(euport_write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	case EU_TYPE_TABLE: /* TODO: change, maybe? */
		_eu_checkreturn(euport_write_string(s, port, "#<table 0x"));
		_eu_checkreturn(euport_write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	default:
		return euport_write_string(s, port, "#<UNKNOWN TYPE>");
	}

	return EU_RESULT_OK;
}

/**
 * @brief Writes a representation of an object to the target textual port. Effectively
 * the implementation of `write`.
 *
 * @remarks Since write-shared is not yet implemented, this relies entirely on
 * write-simple, which means this may not terminate for circular structures.
 *
 * @param s The Europa state.
 * @param port The target port.
 * @param v The target value.
 * @return The result of the operation.
 */
int euport_write(europa* s, struct europa_port* port, struct europa_value* v) {
	/* TODO: implement datum labels and implement proper shared writing procedures */
	return euport_write_simple(s, port, v);
}

/**
 * @brief Same as write, but must use datum labels for values that appear more than
 * once. WARNING: this is not properly implemented, and instead redirects the call to
 * `write-simple`. This is a TODO.
 *
 * @param s The Europa state.
 * @param port The target port.
 * @param v The target value.
 * @param memo A memoization table. (Pass NULL on top-level call.)
 * @return The result of the operation.
 */
int euport_write_shared(europa* s, struct europa_port* port, struct europa_value* v, struct europa_table* memo) {
	return euport_write_simple(s, port, v);
}

/**
 * @brief Writes a representation of the object to a textual port. This is
 * intended to be human-readable, not machine readable.
 *
 * This internally relies on write, which, at the moment, uses write-simple;
 * this means this procedure is not guaranteed to terminate when passed
 * circular structures. Fixing this is a TODO.
 *
 * @param s
 * @param port
 * @param v
 * @return int
 */
int euport_display(europa* s, struct europa_port* port, struct europa_value* v) {
	switch (_euvalue_type(v)) {
	case EU_TYPE_STRING:
		return euport_write_string(s, port, _eustring_text(_euvalue_to_string(v)));
	case EU_TYPE_SYMBOL:
		return euport_write_string(s, port, _eusymbol_text(_euvalue_to_symbol(v)));
	case EU_TYPE_CHARACTER:
		return euport_write_char(s, port, _euvalue_to_char(v));
	default:
		return euport_write(s, port, v);
	}
	return EU_RESULT_OK;
}
