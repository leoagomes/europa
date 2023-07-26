#ifndef __EUROPA_H__
#define __EUROPA_H__

#include "europa/common.h"
#include "europa/int.h"

#include "europa/types.h"

#include "europa/gc.h"
#include "europa/object.h"
#include "europa/table.h"

#include "europa/types.h"

#define _euglobal_gc(g) (&((g)->gc))
#define _euglobal_main(g) ((g)->main)
#define _euglobal_panic(g) ((g)->panic)
#define _euglobal_env(g) ((g)->env)

#define _eu_global(s) ((s)->global)
#define _eu_gc(s) _euglobal_gc(_eu_global(s))
#define _eu_env(s) ((s)->environment)
#define _eu_err(s) ((s)->last_error)
#define _eu_acc(s) (&((s)->accumulator))
#define _struct europa_global_env(s) (_euglobal_env(_eu_global(s)))

#define _eu_reset_err(s) (_eu_err(s) = NULL)

europa* eu_new(europa_realloc f, void* ud, europa_c_callback panic, int* err);
int eu_terminate(europa* s);

int eu_set_error(europa* s, int flags, struct europa_error* nested, void* error_text);
int eu_set_error_nf(europa* s, int flags, struct europa_error* nested, size_t len, const char* fmt, ...);

int eu_do_string(europa* s, void* text, struct europa_value* out);

/* TODO: add a define to keep stdio out */
int eu_do_file(europa* s, const char* filename, struct europa_value* out);

int eu_recover(europa* s, struct europa_error** err);


eu_uinteger eustate_hash(europa* vec);
int eustate_mark(europa* s, europa_gc_mark mark, europa* state);

eu_uinteger euglobal_hash(struct europa_global* gl);
int euglobal_mark(europa* s, europa_gc_mark mark, struct europa_global* state);

#endif /* __EUROPA_H__ */
