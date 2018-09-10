#ifndef __EUROPA_BYTEVECTOR_H__
#define __EUROPA_BYTEVECTOR_H__

#include "europa.h"
#include "eu_int.h"
#include "eu_commons.h"
#include "eu_object.h"

typedef struct europa_bytevector eu_bvector;

struct europa_bytevector {
	EU_OBJ_COMMON_HEADER;

	size_t length;
	eu_byte _data;
};

#define _eubvector_to_obj(v) cast(eu_gcobj*, v)
#define _euobj_to_bvector(o) cast(eu_bvector*, o)

#define _euvalue_to_bvector(v) _euobj_to_bvector((v)->value.object)

#define _eu_makebvector(vptr, v) do {\
		eu_bvector* __bvector__ = (v);\
		(vptr)->type = EU_TYPE_BYTEVECTOR | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eubvector_to_obj((v));\
	} while (0)

/* member access macros */

#define _eubvector_data(v) (&((v)->_data))
#define _eubvector_length(v) ((v)->length)

/* function declarations */

eu_bvector* eubvector_new(europa* s, size_t length, eu_byte* data);

void* eubvector_text(eu_bvector* vec);
eu_integer eubvector_hash(eu_bvector* vec);
eu_integer eubvector_rehash(eu_bvector* vec);
eu_integer eubvector_size(eu_bvector* vec);


#endif
