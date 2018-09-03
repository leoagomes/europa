#ifndef __EUROPA_UTILS_H__
#define __EUROPA_UTILS_H__

#include "eu_commons.h"
#include "eu_int.h"

eu_integer eutil_strb_hash(eu_byte* str, eu_uint len);
eu_integer eutil_cstr_hash(char* str);

int unicodetoutf8(int c);

#endif /* __EUROPA_UTILS_H__ */
