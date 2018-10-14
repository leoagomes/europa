#ifndef __EUROPA_FRAME_H__
#define __EUROPA_FRAME_H__

#include "europa.h"
#include "eu_object.h"
#include "eu_int.h"
#include "eu_commons.h"
#include "eu_pair.h"

typedef struct europa_frame eu_frame;

struct europa_frame {
	EU_OBJ_COMMON_HEADER;

	eu_frame* previous; /*!< previous call frame */
	eu_table* env; /*!< call frame environment */
	eu_pair* rib; /*!< frame value rib */
	eu_value next; /*!< next expression */
};

#define _euframe_to_obj(s) cast(eu_gcobj*, s)
#define _euobj_to_frame(o) cast(eu_frame*, o)

#define _euvalue_to_frame(v) _euobj_to_frame((v)->value.object)
#define _eu_makeframe(vptr, s) do {\
		(vptr)->type = EU_TYPE_CONTINUATION | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euframe_to_obj(s);\
	} while (0)

eu_frame* euframe_new(europa* s, eu_cfunc cfunc, eu_pair* body, eu_pair* formals,
	eu_table* env);

eu_result euframe_mark(europa* s, eu_gcmark mark, eu_frame* cl);
eu_result euframe_destroy(europa* s, eu_frame* cl);
eu_integer euframe_hash(eu_frame* cl);

#endif