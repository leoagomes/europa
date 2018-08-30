#include "eu_port.h"

#include "ports/eu_fport.h"
#include "ports/eu_mport.h"

eu_result euport_mark(eu_gc* gc, eu_gcmark mark, eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_mark(gc, mark, _euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
}

eu_result euport_destroy(eu_gc* gc, eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_destroy(gc, _euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
}

eu_integer euport_hash(europa* s, eu_port* port) {
	switch (port->type) {
		case EU_PORT_TYPE_FILE:
			return eufport_hash(s, _euport_to_fport(port));
		case EU_PORT_TYPE_MEMORY:
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
}

/* internal functions */

#define STATE_PORT_OUT_SWITCH(restype, func) \
eu_result euport_ ## func (europa* s, eu_port* port, restype* out) {\
	if (!s || !port || !out)\
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

STATE_PORT_OUT_SWITCH(int, read_char)
STATE_PORT_OUT_SWITCH(int, peek_char)
STATE_PORT_OUT_SWITCH(eu_value, read_line)
STATE_PORT_OUT_SWITCH(int, char_ready)
STATE_PORT_OUT_SWITCH(eu_value, read_u8)
STATE_PORT_OUT_SWITCH(eu_value, peek_u8)
STATE_PORT_OUT_SWITCH(int, u8_ready)

eu_result euport_read_string(europa* s, eu_port* port, int k, eu_value* out) {
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
}