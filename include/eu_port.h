#ifndef __EUROPA_PORT_H__
#define __EUROPA_PORT_H__

#include "eu_int.h"
#include "eu_object.h"
#include "eu_gc.h"
#include "europa.h"

#define EU_PORT_COMMON_HEAD eu_byte port_type;

#define EU_PORT_INPUT_MASK 0x80
#define EU_PORT_OUTPUT_MASK 0x40
#define EU_PORT_TEXTUAL_MASK 0x20
#define EU_PORT_BINARY_MASK 0x10
#define EU_PORT_TYPE_MASK 0x0F

enum eu_port_type {
	EU_PORT_TYPE_STRING,
	EU_PORT_TYPE_FILE
};

typedef struct eu_port eu_port;

struct eu_port {
	EU_COMMON_HEAD;
	EU_PORT_COMMON_HEAD;
};

#define eu_gcobj2port(obj) cast(eu_port*,(obj))
#define eu_port2gcobj(port) cast(eu_gcobj*, (port))

#define euport_is_input(port) (((port)->port_type & EU_PORT_INPUT_MASK) != 0)
#define euport_is_output(port) (((port)->port_type & EU_PORT_OUTPUT_MASK) != 0)
#define euport_is_textual(port) (((port)->port_type & EU_PORT_TEXTUAL_MASK) != 0)
#define euport_is_binary(port) (((port)->port_type & EU_PORT_BINARY_MASK) != 0)

#define euport_type(port) ((port)->type & EU_PORT_TYPE_MASK)

eu_port* euport_new(europa* s, eu_byte type);
void euport_destroy(europa_gc* gc, eu_port* port);

#endif