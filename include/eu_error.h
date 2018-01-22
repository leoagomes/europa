#ifndef __EUROPA_ERROR_H__
#define __EUROPA_ERROR_H__

#include "eu_int.h"
#include "eu_commons.h"
#include "eu_object.h"
#include "eu_gc.h"
#include "europa.h"

#define EU_ERROR_MSG_LEN 512

enum eu_error_type {
	EU_ERROR_BAD_TYPE_VT,
	EU_ERROR_BAD_ARGUMENT_COUNT,
	EU_ERROR_ARITY_MISMATCH,
};

typedef struct eu_error eu_error;

struct eu_error {
	EU_COMMON_HEAD;

	eu_byte error_type;
	eu_byte gt, et;
	eu_value gv, ev;
	int count;

	char message[EU_ERROR_MSG_LEN];
	/* TODO: add continuation data. */
};

#define euerr_get_message(e) ((e)->message)

#define euerr_size() (sizeof(eu_error))

#define eu_error2gcobj(e) cast(eu_gcobj*,(e))
#define eu_gcobj2error(e) cast(eu_error*,(e))

eu_error* euerr_new(europa* s, const char* fmt, ...);
void euerr_destroy(europa_gc* gc, eu_error* e);

eu_value euerr_tovalue(eu_error* e);

/* TODO: replace bad_value_type by bad_argument_type when applicable */
eu_error* euerr_bad_value_type(europa* s, eu_value given, eu_byte expected);
eu_error* euerr_bad_argument_type(europa* s, const char* to, int arg_index,
	eu_value given, int expected_type);
/* TODO: replace bad_argument_count by arity_mismatch and remove
 * bad_argument_count */
eu_error* euerr_bad_argument_count(europa* s, const char* to, int count);
eu_error* euerr_arity_mismatch(europa* s, const char* name, int ex, int given);

#endif