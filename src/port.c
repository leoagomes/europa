/** Port type operations.
 * 
 * @file port.c
 * @author Leonardo G.
 */
#include "eu_port.h"

#include "ports/eu_fport.h"
#include "ports/eu_mport.h"

eu_result euport_mark(europa* s, eu_gcmark mark, eu_port* port) {
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

eu_result euport_destroy(europa* s, eu_port* port) {
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
eu_result euport_ ## func (europa* s, eu_port* port, restype out) {\
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

STATE_PORT_OUT_SWITCH(int*, read_char)
STATE_PORT_OUT_SWITCH(int*, peek_char)
STATE_PORT_OUT_SWITCH(eu_value*, read_line)
STATE_PORT_OUT_SWITCH(int*, char_ready)
STATE_PORT_OUT_SWITCH(eu_value*, read_u8)
STATE_PORT_OUT_SWITCH(eu_value*, peek_u8)
STATE_PORT_OUT_SWITCH(int*, u8_ready)

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
	return EU_RESULT_ERROR;
}


STATE_PORT_OUT_SWITCH(eu_value, euport_write)
STATE_PORT_OUT_SWITCH(eu_value, euport_write_shared)
STATE_PORT_OUT_SWITCH(eu_value, euport_write_simple)
STATE_PORT_OUT_SWITCH(eu_value, euport_display)
STATE_PORT_OUT_SWITCH(eu_value, euport_newline)
STATE_PORT_OUT_SWITCH(int, euport_write_char)
STATE_PORT_OUT_SWITCH(eu_byte, euport_write_u8)
STATE_PORT_OUT_SWITCH(eu_value, euport_write_bytevector)
STATE_PORT_OUT_SWITCH(eu_value, euport_flush)

