#ifndef __EUROPA_UTILS_H__
#define __EUROPA_UTILS_H__

#include "europa/europa.h"
#include "europa/object.h"
#include "europa/commons.h"
#include "europa/int.h"

/* honestly, this is where things that don't have a home yet go */

eu_uinteger eutil_strb_hash(eu_byte* str, eu_uint len);
eu_uinteger eutil_cstr_hash(const char* str);

int unicodetoutf8(int c);

int eutil_list_length(europa* s, eu_value* v, int* improper);

void* eutil_stdlib_realloclike(void* ud, void* ptr, size_t size);

eu_result eutil_register_standard_library(europa* s);
eu_result eutil_set_standard_ports(europa* s);

#endif /* __EUROPA_UTILS_H__ */
