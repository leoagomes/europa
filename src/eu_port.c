#include "eu_port.h"

#include "ports/eu_fport.h"

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
			return eufport_hash(s, _euport_for_fport(port));
		case EU_PORT_TYPE_MEMORY:
		case EU_PORT_TYPE_VIRTUAL:
			// TODO: implement
		default:
			break;
	}
}