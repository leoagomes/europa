#ifndef __EUROPA_FILE_PORT_H__
#define __EUROPA_FILE_PORT_H__

#include "europa.h"
#include "eu_commons.h"
#include "eu_port.h"

#include <stdio.h>

typedef struct europa_fport eu_fport;

struct europa_fport {
	EU_PORT_COMMON_HEADER;

	FILE* file;
};

#define _eufport_to_port(p) cast(eu_port*, p)
#define _euport_to_fport(p) cast(eu_fport*, p)

eu_fport* eufport_open(europa* s, eu_byte flags, const char* filename);

eu_result eufport_mark(eu_gc* gc, eu_gcmark mark, eu_fport* port);
eu_result eufport_destroy(eu_gc* gc, eu_fport* port);

eu_integer eufport_hash(europa* s, eu_fport* port);

/* internal use functions */
/* input */
eu_result eufport_read_char(europa* s, eu_fport* port, int* out);
eu_result eufport_peek_char(europa* s, eu_fport* port, int* out);
eu_result eufport_read_line(europa* s, eu_fport* port, eu_value* out);
eu_result eufport_char_ready(europa* s, eu_fport* port, int* ready);
eu_result eufport_read_string(europa* s, eu_fport* port, eu_value* out);
eu_byte eufport_read_u8(europa* s, eu_fport* port);
eu_byte eufport_peek_u8(europa* s, eu_fport* port);
eu_bool eufport_u8_ready(europa* s, eu_fport* port);

/* output */

/* language api */

eu_result euapi_fport_read(europa* s);
eu_result euapi_fport_read_char(europa* s);
eu_result euapi_fport_peek_char(europa* s);
eu_result euapi_fport_read_line(europa* s);
eu_result euapi_fport_char_ready(europa* s);
eu_result euapi_fport_read_string(europa* s);
eu_result euapi_fport_read_u8(europa* s);
eu_result euapi_fport_peek_u8(europa* s);
eu_result euapi_fport_u8_ready(europa* s);
eu_result euapi_fport_read_bytevector(europa* s);
eu_result euapi_fport_read_bytevectorB(europa* s);

eu_result euapi_fport_write(europa* s);
eu_result euapi_fport_write_shared(europa* s);
eu_result euapi_fport_write_simple(europa* s);
eu_result euapi_fport_display(europa* s);
eu_result euapi_fport_newline(europa* s);
eu_result euapi_fport_write_char(europa* s);
eu_result euapi_fport_write_u8(europa* s);
eu_result euapi_fport_write_bytevector(europa* s);
eu_result euapi_fport_flush(europa* s);

#endif