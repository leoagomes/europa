#include "eu_bytevector.h"

#include <string.h>

eu_bvector* eubvector_new(europa* s, size_t length, eu_byte* data) {
	eu_bvector* vec;

	if (!s || !data || length <= 0)
		return NULL;

	vec = eugc_new_object(_eu_get_gc(s), EU_TYPE_BYTEVECTOR |
		EU_TYPEFLAG_COLLECTABLE, sizeof(eu_bvector) + length);
	if (vec == NULL)
		return NULL;

	memcpy(_eubvector_data(vec), data, length);
	vec->length = length;

	return vec;
}

eu_byte* eubvector_data(eu_bvector* vec) {
	if (vec == NULL)
		return NULL;
	return _eubvector_data(vec);
}

eu_integer eubvector_hash(eu_bvector* vec) {
	return cast(eu_integer, vec);
}

eu_integer eubvector_length(eu_bvector* vec) {
	if (vec == NULL)
		return -1;
	return _eubvector_length(vec);
}
