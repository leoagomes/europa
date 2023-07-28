#ifndef __EUROPA_ERROR_H__
#define __EUROPA_ERROR_H__

#include "europa/europa.h"
#include "europa/int.h"
#include "europa/common.h"
#include "europa/object.h"

#include "europa/types.h"

enum {
    EU_ERROR_NONE, /* it makes no sense, I know */
    EU_ERROR_READ,
    EU_ERROR_WRITE,
};

struct europa_error* euerror_new(
    europa* s,
    int flags,
    struct europa_string* message,
    struct europa_error* nested
);
void* euerror_message(struct europa_error* err);
eu_uinteger euerror_hash(struct europa_error* err);

#endif
