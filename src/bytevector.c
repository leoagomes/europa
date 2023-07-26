/** Bytevector type implementation.
 *
 * @file bytevector.c
 * @author Leonardo G.
 */
#include "europa/bytevector.h"

#include <string.h>

struct europa_bytevector* eubvector_new(europa* s, eu_integer length, eu_byte* data) {
    struct europa_bytevector* vec;

    if (!s || !data || length <= 0)
        return NULL;

    vec = _euobj_to_bvector(eugc_new_object(s, EU_TYPE_BYTEVECTOR |
        EU_TYPEFLAG_COLLECTABLE, sizeof(struct europa_bytevector) + length));
    if (vec == NULL)
        return NULL;

    memcpy(_eubvector_data(vec), data, length);
    vec->length = length;

    return vec;
}

eu_byte* eubvector_data(struct europa_bytevector* vec) {
    if (vec == NULL)
        return NULL;
    return _eubvector_data(vec);
}

eu_uinteger eubvector_hash(struct europa_bytevector* vec) {
    return cast(eu_integer, vec);
}

eu_integer eubvector_length(struct europa_bytevector* vec) {
    if (vec == NULL)
        return -1;
    return _eubvector_length(vec);
}
