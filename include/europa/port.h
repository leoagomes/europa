#ifndef __EUROPA_PORT_H__
#define __EUROPA_PORT_H__

#include "europa/europa.h"

#include "europa/int.h"
#include "europa/common.h"
#include "europa/object.h"
#include "europa/string.h"
#include "europa/bytevector.h"

/* type definitions */
typedef struct europa_port eu_port;

/* things that are common to all ports */
#define EU_PORT_COMMON_HEADER \
	EU_OBJECT_HEADER \
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
int euport_mark(europa* s, eu_gcmark mark, eu_port* port);
int euport_destroy(europa* s, eu_port* port);
eu_uinteger euport_hash(eu_port* port);

/* internal functions */
/* input */
int euport_read(europa* s, eu_port* port, eu_value* out);
int euport_read_char(europa* s, eu_port* port, int* out);
int euport_peek_char(europa* s, eu_port* port, int* out);
int euport_read_line(europa* s, eu_port* port, eu_value* out);
int euport_char_ready(europa* s, eu_port* port, int* ready);
int euport_read_string(europa* s, eu_port* port, int k, eu_value* out);
int euport_read_u8(europa* s, eu_port* port, eu_value* out);
int euport_peek_u8(europa* s, eu_port* port, eu_value* out);
int euport_u8_ready(europa* s, eu_port* port, int* ready);

/* obs: use memo=NULL for top level call */
int euport_write(europa* s, eu_port* port, eu_value* v);
int euport_write_shared(europa* s, eu_port* port, eu_value* v, eu_table* memo);
int euport_write_simple(europa* s, eu_port* port, eu_value* v);
int euport_display(europa* s, eu_port* port, eu_value* v);
int euport_newline(europa* s, eu_port* port, eu_value* v);
int euport_write_char(europa* s, eu_port* port, int v);
int euport_write_string(europa* s, eu_port* port, void* v);
int euport_write_u8(europa* s, eu_port* port, eu_byte v);
int euport_write_bytevector(europa* s, eu_port* port, eu_bvector* v);
int euport_flush(europa* s, eu_port* port);

/* obs: the following functions aren't to be considered stable */
int euport_write_integer(europa* s, eu_port* port, eu_integer v);
int euport_write_hex_uint(europa* s, eu_port* port, eu_uinteger v);


/* output */

/* language interface */

int euapi_register_port(europa* s);

int euapi_eof_objectQ(europa* s);
int euapi_eof_object(europa* s);

/* input */
int euapi_port_read(europa* s);
int euapi_port_read_char(europa* s);
int euapi_port_peek_char(europa* s);
int euapi_port_read_line(europa* s);
int euapi_port_char_ready(europa* s);
int euapi_port_read_string(europa* s);
int euapi_port_read_u8(europa* s);
int euapi_port_peek_u8(europa* s);
int euapi_port_u8_ready(europa* s);
int euapi_port_read_bytevector(europa* s);
int euapi_port_read_bytevectorB(europa* s);

/* output */
int euapi_port_write(europa* s);
int euapi_port_write_shared(europa* s);
int euapi_port_write_string(europa* s);
int euapi_port_write_simple(europa* s);
int euapi_port_display(europa* s);
int euapi_port_newline(europa* s);
int euapi_port_write_char(europa* s);
int euapi_port_write_u8(europa* s);
int euapi_port_write_bytevector(europa* s);
int euapi_port_flush(europa* s);

/* these still need implementation */
int euapi_call_with_port(europa* s);
int euapi_call_with_input_file(europa* s);
int euapi_call_with_output_file(europa* s);
int euapi_input_portQ(europa* s);
int euapi_output_portQ(europa* s);
int euapi_binary_portQ(europa* s);
int euapi_textual_portQ(europa* s);
int euapi_portQ(europa* s);
int euapi_output_port_openQ(europa* s);
int euapi_input_port_openQ(europa* s);
int euapi_current_input_port(europa* s);
int euapi_current_output_port(europa* s);
int euapi_current_error_port(europa* s);
int euapi_open_input_file(europa* s);
int euapi_open_binary_input_file(europa* s);
int euapi_open_output_file(europa* s);
int euapi_open_binary_output_file(europa* s);
int euapi_close_port(europa* s);
int euapi_close_input_port(europa* s);
int euapi_close_output_port(europa* s);

#endif
