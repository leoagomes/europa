#ifndef __EUROPA_UTILS_H__
#define __EUROPA_UTILS_H__

#include "europa.h"
#include "eu_object.h"
#include "eu_commons.h"
#include "eu_int.h"

eu_uinteger eutil_strb_hash(eu_byte* str, eu_uint len);
eu_uinteger eutil_cstr_hash(const char* str);

int unicodetoutf8(int c);

int eutil_list_length(europa* s, eu_value* v, int* improper);

void* eutil_stdlib_realloclike(void* ud, void* ptr, size_t size);

#endif /* __EUROPA_UTILS_H__ */
