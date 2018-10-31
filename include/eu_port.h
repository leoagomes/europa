#ifndef __EUROPA_PORT_H__
#define __EUROPA_PORT_H__

#include "europa.h"

#include "eu_int.h"
#include "eu_commons.h"
#include "eu_object.h"
#include "eu_string.h"
#include "eu_bytevector.h"

/* type definitions */
typedef struct europa_port eu_port;

/* things that are common to all ports */
#define EU_PORT_COMMON_HEADER \
	EU_OBJ_COMMON_HEADER; \
	eu_byte flags; \
	eu_byte type

#define EU_PORT_FLAG_INPUT 0x01
#define EU_PORT_FLAG_OUTPUT 0x02
#define EU_PORT_FLAG_TEXTUAL 0x04
#define EU_PORT_FLAG_BINARY 0x08

enum eu_port_type {
	EU_PORT_TYPE_FILE,
	EU_PORT_TYPE_MEMORY,
	EU_PORT_TYPE_VIRTUAL,
};

struct europa_port {
	EU_PORT_COMMON_HEADER;
};


#define _euobj_to_port(o) cast(eu_port*, o)
#define _euport_to_obj(v) cast(eu_object*, v)

#define _euvalue_to_port(v) _euobj_to_port(&((v)->value.object))
#define _eu_makeport(vptr, v) do {\
		(vptr)->type = EU_TYPE_PORT | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euport_to_obj(v);\
	} while (0)


/* function declarations */
eu_result euport_mark(europa* s, eu_gcmark mark, eu_port* port);
eu_result euport_destroy(europa* s, eu_port* port);
eu_uinteger euport_hash(eu_port* port);

/* internal functions */
/* input */
eu_result euport_read(europa* s, eu_port* port, eu_value* out);
eu_result euport_read_char(europa* s, eu_port* port, int* out);
eu_result euport_peek_char(europa* s, eu_port* port, int* out);
eu_result euport_read_line(europa* s, eu_port* port, eu_value* out);
eu_result euport_char_ready(europa* s, eu_port* port, int* ready);
eu_result euport_read_string(europa* s, eu_port* port, int k, eu_value* out);
eu_result euport_read_u8(europa* s, eu_port* port, eu_value* out);
eu_result euport_peek_u8(europa* s, eu_port* port, eu_value* out);
eu_result euport_u8_ready(europa* s, eu_port* port, int* ready);

/* obs: use memo=NULL for top level call */
eu_result euport_write(europa* s, eu_port* port, eu_value* v);
eu_result euport_write_shared(europa* s, eu_port* port, eu_value* v, eu_table* memo);
eu_result euport_write_simple(europa* s, eu_port* port, eu_value* v);
eu_result euport_display(europa* s, eu_port* port, eu_value* v);
eu_result euport_newline(europa* s, eu_port* port, eu_value* v);
eu_result euport_write_char(europa* s, eu_port* port, int v);
eu_result euport_write_string(europa* s, eu_port* port, void* v);
eu_result euport_write_u8(europa* s, eu_port* port, eu_byte v);
eu_result euport_write_bytevector(europa* s, eu_port* port, eu_bvector* v);
eu_result euport_flush(europa* s, eu_port* port);


/* output */

/* language interface */

/* input */
eu_result euapi_port_read(europa* s);
eu_result euapi_port_read_char(europa* s);
eu_result euapi_port_peek_char(europa* s);
eu_result euapi_port_read_line(europa* s);
eu_result euapi_port_char_ready(europa* s);
eu_result euapi_port_read_string(europa* s);
eu_result euapi_port_read_u8(europa* s);
eu_result euapi_port_peek_u8(europa* s);
eu_result euapi_port_u8_ready(europa* s);
eu_result euapi_port_read_bytevector(europa* s);
eu_result euapi_port_read_bytevectorB(europa* s);

/* output */
eu_result euapi_port_write(europa* s);
eu_result euapi_port_write_shared(europa* s);
eu_result euapi_port_write_simple(europa* s);
eu_result euapi_port_display(europa* s);
eu_result euapi_port_newline(europa* s);
eu_result euapi_port_write_char(europa* s);
eu_result euapi_port_write_u8(europa* s);
eu_result euapi_port_write_bytevector(europa* s);
eu_result euapi_port_flush(europa* s);


#endif
