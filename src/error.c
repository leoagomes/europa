#include "europa/error.h"

#include <string.h>

#include "utf8.h"

struct europa_error* euerror_new(
    europa* s,
    int flags,
    struct europa_string* message,
    struct europa_error* nested
) {
    struct europa_error* err;
    size_t text_size;

    if (!s || !message)
        return NULL;

    err = eugc_new_object(s, EU_TYPEFLAG_COLLECTABLE | EU_TYPE_ERROR, sizeof(*err));
    if (err == NULL)
        return NULL;

    err->flags = flags;
    err->message = message;
    err->nested_error = nested;

    return err;
}

void* euerror_message(struct europa_error* err) {
    return err->message->text;
}

eu_uinteger euerror_hash(struct europa_error* err) {
    return cast(eu_uinteger, err);
}
