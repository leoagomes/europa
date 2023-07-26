#ifndef __EUROPA_BYTEVECTOR_H__
#define __EUROPA_BYTEVECTOR_H__

#include "europa/europa.h"
#include "europa/int.h"
#include "europa/common.h"
#include "europa/object.h"

#include "europa/types.h"

#define _eubvector_to_obj(v) cast(struct europa_object*, v)
#define _euobj_to_bvector(o) cast(struct europa_bytevector*, o)

#define _euvalue_to_bvector(v) _euobj_to_bvector((v)->value.object)

#define _eu_makebvector(vptr, v) do {\
		struct europa_bytevector* __bvector__ = (v);\
		(vptr)->type = EU_TYPE_BYTEVECTOR | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eubvector_to_obj((v));\
	} while (0)

/* member access macros */

#define _eubvector_data(v) (&((v)->data))
#define _eubvector_length(v) ((v)->length)
#define _eubvector_ref(v, i) (((v)->data)[(i)])
#define _eubvector_set(v, i, value) ((&((v)->_data))[(i)] = (value))

/* function declarations */

struct europa_bytevector* eubvector_new(europa* s, eu_integer length, eu_byte* data);

void* eubvector_text(struct europa_bytevector* vec);
eu_uinteger eubvector_hash(struct europa_bytevector* vec);
eu_integer eubvector_rehash(struct europa_bytevector* vec);
eu_integer eubvector_size(struct europa_bytevector* vec);

#endif
