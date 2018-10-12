#ifndef __EUROPA_CLOSURE_H__
#define __EUROPA_CLOSURE_H__

#include "europa.h"
#include "eu_gc.h"
#include "eu_object.h"
#include "eu_table.h"
#include "eu_pair.h"

typedef struct europa_closure eu_closure;

struct europa_closure {
	EU_OBJ_COMMON_HEADER;

	eu_cfunc* cf; /*!< C function closure */

	eu_pair* body; /*!< function body */
	eu_pair* formals; /*!< formal symbols */
	eu_table* env; /*!< declaration environment */
};

#define _euclosure_to_obj(s) cast(eu_gcobj*, s)
#define _euobj_to_closure(o) cast(eu_closure*, o)

#define _euvalue_to_closure(v) _euobj_to_closure((v)->value.object)
#define _euclosure_to_value(v) { \
	.type = EU_TYPE_STRING, \
	.value = { .object = (s) }}

#define _eu_makeclosure(vptr, s) do {\
		(vptr)->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euclosure_to_obj(s);\
	} while (0)

eu_closure* eucl_new(europa* s, eu_cfunc cfunc, eu_pair* body, eu_pair* formals,
	eu_table* env);

eu_result eucl_mark(europa* s, eu_gcmark mark, eu_closure* cl);
eu_result eucl_destroy(europa* s, eu_closure* cl);
eu_integer eucl_hash(eu_closure* cl);

#endif