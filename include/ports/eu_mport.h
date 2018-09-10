#ifndef __EUROPA_MEMORY_PORT_H__
#define __EUROPA_MEMORY_PORT_H__

#include "europa.h"
#include "eu_commons.h"
#include "eu_object.h"
#include "eu_port.h"

#include <stdio.h>

#define EU_GROW_SIZE

typedef struct europa_mport eu_mport;

struct europa_mport {
	EU_PORT_COMMON_HEADER;

	eu_integer rpos, wpos;
	eu_byte* next;

	size_t size;
	eu_byte* mem;
};

#define _eumport_to_port(p) cast(eu_port*, p)
#define _euport_to_mport(p) cast(eu_mport*, p)
#define _eumport_to_obj(p) cast(eu_object*, p)
#define _euobj_to_mport(p) cast(eu_mport*, p)

eu_mport* eumport_from_str(europa* s, eu_byte flags, void* str);

eu_result eumport_mark(eu_gc* gc, eu_gcmark mark, eu_mport* port);
eu_result eumport_destroy(eu_gc* gc, eu_mport* port);

eu_integer eumport_hash(europa* s, eu_mport* port);

/* internal use functions */
/* input */
eu_result eumport_read_char(europa* s, eu_mport* port, int* out);
eu_result eumport_peek_char(europa* s, eu_mport* port, int* out);
eu_result eumport_read_line(europa* s, eu_mport* port, eu_value* out);
eu_result eumport_char_ready(europa* s, eu_mport* port, int* ready);
eu_result eumport_read_string(europa* s, eu_mport* port, int k, eu_value* out);
eu_result eumport_read_u8(europa* s, eu_mport* port, eu_value* out);
eu_result eumport_peek_u8(europa* s, eu_mport* port, eu_value* out);
eu_result eumport_u8_ready(europa* s, eu_mport* port, int* ready);

/* output */

#endif