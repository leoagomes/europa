#ifndef __EUROPA_H__
#define __EUROPA_H__

#include "eu_commons.h"
#include "eu_int.h"

#include "eu_gc.h"
#include "eu_object.h"
#include "eu_table.h"

typedef struct europa_frame eu_frame;
typedef struct europa_pair eu_pair;
typedef struct europa_error eu_error;

typedef struct europa_closure eu_closure;
typedef struct europa_continuation eu_continuation;

typedef struct europa_global eu_global;
typedef struct europa europa;
typedef int (*eu_cfunc)(europa* s);

typedef struct europa_table eu_table;
struct europa_global {
	EU_OBJ_COMMON_HEADER;
	eu_gc gc; /*!< global state GC */
	eu_cfunc panic; /*!< global panic function */
	europa* main; /*!< the main state */
	eu_table* internalized; /*!< internalized strings and symbols */
	eu_table* env; /*!< global environment */
};

struct europa_jmplist;

struct europa {
	EU_OBJ_COMMON_HEADER;
	eu_byte level; /*!< current continuation level */
	eu_byte status; /*!< current execution status */

	eu_global* global; /*!< associated global state */
	struct europa_jmplist* error_jmp; /*!< error jump buf */

	eu_error* err; /*!< last computation error */

	/* execution state */
	unsigned int pc; /*!< program counter */
	eu_closure* ccl; /*!< current running closure */
	eu_value acc; /*!< the accumulator */
	eu_table* env; /*!< current environment */
	eu_value rib; /*!< argument rib */
	eu_value* rib_lastpos; /*!< last rib position */
	eu_continuation* previous; /*!< previous continuation */
};

#define _euglobal_gc(g) (&((g)->gc))
#define _euglobal_main(g) ((g)->main)
#define _euglobal_panic(g) ((g)->panic)

#define _eu_global(s) ((s)->global)
#define _eu_gc(s) _euglobal_gc(_eu_global(s))
#define _eu_env(s) ((s)->env)
#define _eu_err(s) ((s)->err)
#define _eu_acc(s) (&((s)->acc))

#define _eu_reset_err(s) (_eu_err(s) = NULL)

europa* eu_new(eu_realloc f, void* ud, eu_cfunc panic, eu_result* err);
eu_result eu_terminate(europa* s);

eu_result eu_set_error(europa* s, int flags, eu_error* nested, void* error_text);
eu_result eu_set_error_nf(europa* s, int flags, eu_error* nested, size_t len, const char* fmt, ...);

eu_result eu_do_string(europa* s, void* text, eu_value* out);

/* TODO: add a define to keep stdio out */
eu_result eu_do_file(europa* s, const char* filename, eu_value* out);


#endif /* __EUROPA_H__ */
