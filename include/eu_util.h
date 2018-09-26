#ifndef __EUROPA_UTILS_H__
#define __EUROPA_UTILS_H__

#include "eu_commons.h"
#include "eu_int.h"

eu_uinteger eutil_strb_hash(eu_byte* str, eu_uint len);
eu_uinteger eutil_cstr_hash(const char* str);

int unicodetoutf8(int c);

#endif /* __EUROPA_UTILS_H__ */
