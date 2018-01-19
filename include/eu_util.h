#ifndef __EUROPA_UTILS_H__
#define __EUROPA_UTILS_H__

#include "eu_commons.h"
#include "eu_int.h"

unsigned long eutl_strb_hash(eu_byte* str, eu_uint len);
unsigned long eutl_cstr_hash(char* str);

#endif /* __EUROPA_UTILS_H__ */