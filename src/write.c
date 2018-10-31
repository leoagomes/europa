#include <stdio.h>

#include "eu_object.h"
#include "eu_string.h"
#include "europa.h"
#include "eu_port.h"
#include "eu_error.h"
#include "eu_number.h"
#include "eu_character.h"
#include "eu_util.h"
#include "eu_symbol.h"
#include "eu_pair.h"
#include "eu_bytevector.h"
#include "eu_vector.h"
#include "eu_table.h"

#include "utf8.h"


eu_result euport_write(europa* s, eu_port* port, eu_value* v);
eu_result euport_write_shared(europa* s, eu_port* port, eu_value* v);

#define NUMBUF_SIZE 128
eu_result write_integer(europa* s, eu_port* port, eu_integer v) {
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

eu_result write_hex_uint(europa* s, eu_port* port, eu_uinteger v) {
	char d[NUMBUF_SIZE];
	int pos, tv;
	eu_uinteger val;

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

eu_result write_string_literal(europa* s, eu_port* port, void* text) {
	void *next;
	int c;

	/* write opening " */
	_eu_checkreturn(euport_write_char(s, port, '"'));

	next = text;
	while (next != NULL) {
		next = utf8codepoint(next, &c);

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

	while (text != NULL) {
		text = utf8codepoint(text, &c);

		if (c > 0xFF)
			return 0;
	}
	return 1;
}

eu_result write_symbol_literal(europa* s, eu_port* port, void* text) {
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

eu_result write_bytevector(europa* s, eu_port* port, eu_bvector* bv) {
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
		_eu_checkreturn(write_integer(s, port, _eubvector_ref(bv, i)));
	}
	/* writen ending ')' */
	_eu_checkreturn(euport_write_char(s, port, ')'));

	return EU_RESULT_OK;
}

eu_result write_char_literal(europa* s, eu_port* port, int c) {

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

eu_result write_real(europa* s, eu_port* port, eu_real v) {
	char buf[NUMBUF_SIZE];

	/* TODO: implement this in a way that does not depend on stdio */
	snprintf(buf, sizeof(buf), "%lf", v);
	euport_write_string(s, port, buf);

	return EU_RESULT_OK;
}

eu_result write_number(europa* s, eu_port* port, eu_value* num) {
	if (_eunum_is_exact(num))
		return write_integer(s, port, _eunum_i(num));
	else
		return write_real(s, port, _eunum_r(num));

	return EU_RESULT_OK;
}

eu_result write_pair_simple(europa* s, eu_port* port, eu_value* p) {
	eu_value* v;

	/* write opening parenthesis ( */
	_eu_checkreturn(euport_write_char(s, port, '('));

	/* write all 'car's */
	for (v = p; _euvalue_is_pair(v); v = _eupair_tail(_euvalue_to_pair(v))) {
		if (v != p) { /* add a space character */
			_eu_checkreturn(euport_write_char(s, port, ' '));
		}
		_eu_checkreturn(euport_write(s, port, v));
	}

	/* check if improper list */
	if (!_euvalue_is_null(v)) {
		_eu_checkreturn(euport_write_string(s, port, " . "));
		_eu_checkreturn(euport_write(s, port, v));
	}

	/* write closing parenthesis ) */
	_eu_checkreturn(euport_write_char(s, port, ')'));

	return EU_RESULT_OK;
}

eu_result write_vector_simple(europa* s, eu_port* port, eu_vector* vec) {
	int i;

	/* write #( prefix */
	_eu_checkreturn(euport_write_string(s, port, "#("));

	/* print vector elements */
	for (i = 0; i < _euvector_length(vec); i++) {
		_eu_checkreturn(euport_write(s, port, _euvector_ref(vec, i)));
	}

	/* write closing parenthesis ) */
	_eu_checkreturn(euport_write_char(s, port, ')'));

	return EU_RESULT_OK;
}

eu_result euport_write_simple(europa* s, eu_port* port, eu_value* v) {

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
		_eu_checkreturn(write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	case EU_TYPE_CONTINUATION:
		_eu_checkreturn(euport_write_string(s, port, "#<continuation 0x"));
		_eu_checkreturn(write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
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
		_eu_checkreturn(write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	case EU_TYPE_PORT:
		_eu_checkreturn(euport_write_string(s, port, "#<port>"));
		break;

	case EU_TYPE_PROTO:
		_eu_checkreturn(euport_write_string(s, port, "#<prototype 0x"));
		_eu_checkreturn(write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	case EU_TYPE_TABLE: /* TODO: change, maybe? */
		_eu_checkreturn(euport_write_string(s, port, "#<table 0x"));
		_eu_checkreturn(write_hex_uint(s, port, cast(eu_uinteger, v->value.object)));
		return euport_write_char(s, port, '>');

	default:
		break;
	}

	return EU_RESULT_OK;
}

eu_result euport_display(europa* s, eu_port* port, eu_value* v) {
	switch (_euvalue_type(v)) {
	case EU_TYPE_STRING:
		return euport_write_string(s, port, _eustring_text(_euvalue_to_string(v)));
	case EU_TYPE_SYMBOL:
		return euport_write_string(s, port, _eusymbol_text(_euvalue_to_symbol(v)));
	case EU_TYPE_CHARACTER:
		return euport_write_char(s, port, _euvalue_to_char(v));
	default:
		break;
	}
	return EU_RESULT_OK;
}