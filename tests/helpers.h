#ifndef __AUX_H__
#define __AUX_H__

#include "europa.h"
#include "eu_rt.h"

void* rlike(void* ud, void* ptr, unsigned long long size);
europa* bootstrap_default_instance(void);
void terminate_default_instance(europa* s);

void print_value(europa* s, eu_value* v);
void print_valueln(europa* s, eu_value* v);
void disas_closure(europa* s, eu_closure* cl);
void disas_proto(europa* s, eu_proto* proto);

#endif