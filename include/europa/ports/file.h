#ifndef __EUROPA_FILE_PORT_H__
#define __EUROPA_FILE_PORT_H__

#include "europa/europa.h"
#include "europa/common.h"
#include "europa/object.h"
#include "europa/port.h"

#include <stdio.h>

#define _eufport_to_port(p) cast(struct europa_port*, p)
#define _euport_to_fport(p) cast(struct europa_fport*, p)
#define _eufport_to_obj(p) cast(struct europa_object*, p)
#define _euobj_to_fport(p) cast(struct europa_fport*, p)

struct europa_file_port* eufport_open(europa* s, eu_byte flags, const char* filename);
struct europa_file_port* eufport_from_file(europa* s, eu_byte flags, FILE* file);

int eufport_mark(europa* s, europa_gc_mark mark, struct europa_file_port* port);
int eufport_destroy(europa* s, struct europa_file_port* port);

eu_uinteger eufport_hash(struct europa_file_port* port);

/* internal use functions */
/* input */
int eufport_read_char(europa* s, struct europa_file_port* port, int* out);
int eufport_peek_char(europa* s, struct europa_file_port* port, int* out);
int eufport_read_line(europa* s, struct europa_file_port* port, struct europa_value* out);
int eufport_char_ready(europa* s, struct europa_file_port* port, int* ready);
int eufport_read_string(europa* s, struct europa_file_port* port, int k, struct europa_value* out);
int eufport_read_u8(europa* s, struct europa_file_port* port, struct europa_value* out);
int eufport_peek_u8(europa* s, struct europa_file_port* port, struct europa_value* out);
int eufport_u8_ready(europa* s, struct europa_file_port* port, int* ready);

int eufport_newline(europa* s, struct europa_file_port* port, struct europa_value* v);
int eufport_write_char(europa* s, struct europa_file_port* port, int v);
int eufport_write_string(europa* s, struct europa_file_port* port, void* v);
int eufport_write_u8(europa* s, struct europa_file_port* port, eu_byte v);
int eufport_write_bytevector(europa* s, struct europa_file_port* port, struct europa_bytevector* v);
int eufport_flush(europa* s, struct europa_file_port* port);

/* output */

#endif
