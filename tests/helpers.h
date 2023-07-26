#ifndef __AUX_H__
#define __AUX_H__

#include "europa.h"

#include "munit.h"

#define assert_ok(exp) munit_assert_int(exp,==,EU_RESULT_OK)
#define assertv_type(vptr,type) munit_assert_int(_euvalue_type(vptr),==,type)
#define assertv_int(vptr,op,val) munit_assert_int((vptr)->value.i,op,val)
#define assertv_real(vptr,op,val) munit_assert_double((vptr)->value.r,op,val)
#define assertv_true(vptr) munit_assert_true((vptr)->value.boolean)
#define assertv_false(vptr) munit_assert_false((vptr)->value.boolean)
#define assertv_char(vptr,op,val) munit_assert_int((vptr)->value.character,op,val)
#define assertv_string_equal(vptr, str) munit_assert_string_equal(_eustring_text(_euvalue_to_string(vptr)), str)
#define assertv_symbol_equal(vptr, str) munit_assert_string_equal(_eusymbol_text(_euvalue_to_symbol(vptr)), str)

void* rlike(void* ud, void* ptr, size_t size);
europa* bootstrap_default_instance(void);
void terminate_default_instance(europa* s);

void printv(europa* s, struct europa_value* v);
void printvln(europa* s, struct europa_value* v);
void printobj(europa* s, struct europa_object* obj);
void printobjln(europa* s, struct europa_object* obj);

void disas_closure(europa* s, struct europa_closure* cl);
void disas_proto(europa* s, struct europa_prototype* proto);
void disas_inst(europa* s, struct europa_prototype* proto, eu_instruction inst);

#endif