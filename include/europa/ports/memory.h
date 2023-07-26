#ifndef __EUROPA_MEMORY_PORT_H__
#define __EUROPA_MEMORY_PORT_H__

#include "europa/europa.h"
#include "europa/common.h"
#include "europa/object.h"
#include "europa/port.h"
#include "europa/bytevector.h"

#include <stdio.h>

#define EU_GROW_SIZE

#define _eumport_to_port(p) cast(struct europa_port*, p)
#define _euport_to_mport(p) cast(struct europa_mport*, p)
#define _eumport_to_obj(p) cast(struct europa_object*, p)
#define _euobj_to_mport(p) cast(struct europa_mport*, p)

struct europa_mport* eumport_from_str(europa* s, eu_byte flags, void* str);

int eumport_mark(europa* s, europa_gc_mark mark, struct europa_mport* port);
int eumport_destroy(europa* s, struct europa_mport* port);

eu_uinteger eumport_hash(struct europa_mport* port);

/* internal use functions */
/* input */
int eumport_read_char(europa* s, struct europa_mport* port, int* out);
int eumport_peek_char(europa* s, struct europa_mport* port, int* out);
int eumport_read_line(europa* s, struct europa_mport* port, struct europa_value* out);
int eumport_char_ready(europa* s, struct europa_mport* port, int* ready);
int eumport_read_string(europa* s, struct europa_mport* port, int k, struct europa_value* out);
int eumport_read_u8(europa* s, struct europa_mport* port, struct europa_value* out);
int eumport_peek_u8(europa* s, struct europa_mport* port, struct europa_value* out);
int eumport_u8_ready(europa* s, struct europa_mport* port, int* ready);

int eumport_write(europa* s, struct europa_mport* port, struct europa_value* v);
int eumport_write_shared(europa* s, struct europa_mport* port, struct europa_value* v);
int eumport_write_simple(europa* s, struct europa_mport* port, struct europa_value* v);
int eumport_display(europa* s, struct europa_mport* port, struct europa_value* v);
int eumport_newline(europa* s, struct europa_mport* port, struct europa_value* v);
int eumport_write_char(europa* s, struct europa_mport* port, int v);
int eumport_write_u8(europa* s, struct europa_mport* port, eu_byte v);
int eumport_write_string(europa* s, struct europa_mport* port, void* str);
int eumport_write_bytevector(europa* s, struct europa_mport* port, struct europa_bytevector* v);
int eumport_flush(europa* s, struct europa_mport* port);

/* output */

#endif
