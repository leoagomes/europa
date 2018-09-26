#ifndef __EUROPA_FILE_PORT_H__
#define __EUROPA_FILE_PORT_H__

#include "europa.h"
#include "eu_commons.h"
#include "eu_object.h"
#include "eu_port.h"

#include <stdio.h>

typedef struct europa_fport eu_fport;

struct europa_fport {
	EU_PORT_COMMON_HEADER;

	FILE* file;
};

#define _eufport_to_port(p) cast(eu_port*, p)
#define _euport_to_fport(p) cast(eu_fport*, p)
#define _eufport_to_obj(p) cast(eu_object*, p)
#define _euobj_to_fport(p) cast(eu_fport*, p)

eu_fport* eufport_open(europa* s, eu_byte flags, const char* filename);

eu_result eufport_mark(europa* s, eu_gcmark mark, eu_fport* port);
eu_result eufport_destroy(europa* s, eu_fport* port);

eu_integer eufport_hash(eu_fport* port);

/* internal use functions */
/* input */
eu_result eufport_read_char(europa* s, eu_fport* port, int* out);
eu_result eufport_peek_char(europa* s, eu_fport* port, int* out);
eu_result eufport_read_line(europa* s, eu_fport* port, eu_value* out);
eu_result eufport_char_ready(europa* s, eu_fport* port, int* ready);
eu_result eufport_read_string(europa* s, eu_fport* port, int k, eu_value* out);
eu_result eufport_read_u8(europa* s, eu_fport* port, eu_value* out);
eu_result eufport_peek_u8(europa* s, eu_fport* port, eu_value* out);
eu_result eufport_u8_ready(europa* s, eu_fport* port, int* ready);

/* output */

#endif
