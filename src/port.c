/** Port type operations.
 *
 * @file port.c
 * @author Leonardo G.
 */
#include "europa/port.h"

#include "europa/ccont.h"
#include "europa/number.h"
#include "europa/character.h"
#include "europa/bytevector.h"

#include "europa/ports/file.h"
#include "europa/ports/memory.h"

int euport_mark(europa* s, eu_gcmark mark, eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_mark(s, mark, _euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
			return eumport_mark(s, mark, _euport_to_mport(port));
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
	return EU_RESULT_ERROR;
}

int euport_destroy(europa* s, eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_destroy(s, _euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
			return eumport_destroy(s, _euport_to_mport(port));
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
	return EU_RESULT_ERROR;
}

eu_uinteger euport_hash(eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_hash(_euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
			return eumport_hash(_euport_to_mport(port));
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
	return EU_RESULT_ERROR;
}

/* internal functions */

#define STATE_PORT_OUT_SWITCH(restype, func) \
int euport_ ## func (europa* s, eu_port* port, restype out) {\
	if (!s || !port)\
		return EU_RESULT_NULL_ARGUMENT;\
	switch(port->type) {\
		case EU_PORT_TYPE_FILE:\
			return eufport_ ## func (s, _euport_to_fport(port), out);\
		case EU_PORT_TYPE_MEMORY:\
			return eumport_ ## func (s, _euport_to_mport(port), out);\
		case EU_PORT_TYPE_VIRTUAL:\
			/* TODO: implement */\
		default:\
			break;\
	}\
	return EU_RESULT_INVALID;\
	\
}

STATE_PORT_OUT_SWITCH(int*, read_char)
STATE_PORT_OUT_SWITCH(int*, peek_char)
STATE_PORT_OUT_SWITCH(eu_value*, read_line)
STATE_PORT_OUT_SWITCH(int*, char_ready)
STATE_PORT_OUT_SWITCH(eu_value*, read_u8)
STATE_PORT_OUT_SWITCH(eu_value*, peek_u8)
STATE_PORT_OUT_SWITCH(int*, u8_ready)

int euport_read_string(europa* s, eu_port* port, int k, eu_value* out) {
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_read_string(s, _euport_to_fport(port), k, out);
		case EU_PORT_TYPE_MEMORY:
			return eumport_read_string(s, _euport_to_mport(port), k, out);
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
	return EU_RESULT_ERROR;
}

STATE_PORT_OUT_SWITCH(eu_value*, newline)
STATE_PORT_OUT_SWITCH(int, write_char)
STATE_PORT_OUT_SWITCH(eu_byte, write_u8)
STATE_PORT_OUT_SWITCH(eu_bvector*, write_bytevector)
STATE_PORT_OUT_SWITCH(void*, write_string)

int euport_flush(europa* s, eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_flush(s, _euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
			return eumport_flush(s, _euport_to_mport(port));
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
	return EU_RESULT_ERROR;
}


int euapi_register_port(europa* s) {
	eu_table* env;

	env = s->env;

	_eu_checkreturn(eucc_define_cclosure(s, env, env, "eof-object?", euapi_eof_objectQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "eof-object", euapi_eof_object));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "read", euapi_port_read));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "read-char", euapi_port_read_char));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "peek-char", euapi_port_peek_char));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "read-line", euapi_port_read_line));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "char-ready?", euapi_port_char_ready));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "read-string", euapi_port_read_string));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "read-u8", euapi_port_read_u8));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "peek-u8", euapi_port_peek_u8));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "u8-ready?", euapi_port_u8_ready));

	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write", euapi_port_write));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write-shared", euapi_port_write_shared));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write-simple", euapi_port_write_simple));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "display", euapi_port_display));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "newline", euapi_port_newline));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write-char", euapi_port_write_char));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write-string", euapi_port_write_string));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write-u8", euapi_port_write_u8));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "write-bytevector", euapi_port_write_bytevector));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "flush-output-port", euapi_port_flush));

	return EU_RESULT_OK;
}

int euapi_eof_objectQ(europa* s) {
	eu_value* obj;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, obj, 0);

	_eu_makebool(_eucc_return(s), _euvalue_is_type(obj, EU_TYPE_EOF));
	return EU_RESULT_OK;
}

int euapi_eof_object(europa* s) {
	_eu_makeeof(_eucc_return(s));
	return EU_RESULT_OK;
}

int euapi_port_read(europa* s) {
	eu_port* p;
	eu_value *port;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	return euport_read(s, p, _eucc_return(s));
}

int euapi_port_read_char(europa* s) {
	eu_port* p;
	eu_value *port;
	int c;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_checkreturn(euport_read_char(s, p, &c));

	_eu_makechar(_eucc_return(s), c);
	return EU_RESULT_OK;
}

int euapi_port_peek_char(europa* s) {
	eu_port* p;
	eu_value *port;
	int c;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_checkreturn(euport_peek_char(s, p, &c));
	_eu_makechar(_eucc_return(s), c);
	return EU_RESULT_OK;
}

int euapi_port_read_line(europa* s) {
	eu_port* p;
	eu_value *port;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	return euport_read_line(s, p, _eucc_return(s));
}

int euapi_port_char_ready(europa* s) {
	eu_port* p;
	eu_value *port;
	int ready;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_checkreturn(euport_char_ready(s, p, &ready));
	_eu_makebool(_eucc_return(s), ready);
	return EU_RESULT_OK;
}

int euapi_port_read_string(europa* s) {
	eu_port* p;
	eu_value *k, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument_type(s, k, 0, EU_TYPE_NUMBER);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	return euport_read_string(s, p, _eunum_to_int(k), _eucc_return(s));
}

int euapi_port_read_u8(europa* s) {
	eu_port* p;
	eu_value *k, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument_type(s, k, 0, EU_TYPE_NUMBER);

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	return euport_read_u8(s, p, _eucc_return(s));
}

int euapi_port_peek_u8(europa* s) {
	eu_port* p;
	eu_value *k, *port;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	return euport_peek_u8(s, p, _eucc_return(s));
}

int euapi_port_u8_ready(europa* s) {
	eu_port* p;
	eu_value *port;
	int ready;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->input_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_checkreturn(euport_u8_ready(s, p, &ready));
	_eu_makebool(_eucc_return(s), ready);
	return EU_RESULT_OK;
}

int euapi_port_write(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument(s, obj, 0);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write(s, p, obj);
}

int euapi_port_write_shared(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument(s, obj, 0);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write_shared(s, p, obj, NULL);
}

int euapi_port_write_string(europa* s) {
	eu_port* p;
	eu_value *string, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument_type(s, string, 0, EU_TYPE_STRING);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write_string(s, p, _eustring_text(_euvalue_to_string(string)));
}

int euapi_port_write_simple(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument(s, obj, 0);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write_simple(s, p, obj);
}

int euapi_port_display(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument(s, obj, 0);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_display(s, p, obj);
}

int euapi_port_newline(europa* s) {
	eu_port* p;
	eu_value *port;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_newline(s, p, _eucc_return(s));
}

int euapi_port_write_char(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument_type(s, obj, 0, EU_TYPE_CHARACTER);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write_char(s, p, _euvalue_to_char(obj));
}

int euapi_port_write_u8(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument_type(s, obj, 0, EU_TYPE_CHARACTER);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write_u8(s, p, _eunum_to_int(obj));
}

int euapi_port_write_bytevector(europa* s) {
	eu_port* p;
	eu_value *obj, *port;

	_eucc_arity_improper(s, 1);
	_eucc_argument_type(s, obj, 0, EU_TYPE_BYTEVECTOR);

	/* get port argument */
	port = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_write_bytevector(s, p, _euvalue_to_bvector(obj));
}

int euapi_port_flush(europa* s) {
	eu_port* p;
	eu_value *port;

	/* get port argument */
	port = _eucc_arguments(s);
	if (_euvalue_is_null(port)) {
		p = s->output_port;
	} else {
		if (_euvalue_is_pair(port))
			port = _eupair_head(_euvalue_to_pair(port));

		_eucc_check_type(s, port, "argument", EU_TYPE_PORT);
		p = _euvalue_to_port(port);
	}

	/* at this point p is a valid port */
	_eu_makenull(_eucc_return(s));
	return euport_flush(s, p);
}
