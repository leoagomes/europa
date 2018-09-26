#ifndef __EUROPA_RUNTIME_H__
#define __EUROPA_RUNTIME_H__

#include "europa.h"

typedef void (*eu_pfunc)(europa* s, void* ud);

eu_result eurt_runcprotected(europa* s, eu_cfunc f);

#endif