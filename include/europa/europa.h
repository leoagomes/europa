#ifndef __EUROPA_H__
#define __EUROPA_H__

#include "europa/common.h"
#include "europa/int.h"

#include "europa/gc.h"
#include "europa/object.h"
#include "europa/table.h"

typedef struct europa_frame eu_frame;
typedef struct europa_pair eu_pair;
typedef struct europa_error eu_error;
typedef struct europa_port eu_port;

typedef struct europa_closure eu_closure;
typedef struct europa_continuation eu_continuation;

typedef struct europa_global eu_global;
typedef struct europa europa;
typedef int (*eu_cfunc)(europa* s);

typedef struct europa_table eu_table;
struct europa_global {
	EU_OBJECT_HEADER
	eu_gc gc; /*!< global state GC */
	eu_cfunc panic; /*!< global panic function */
	europa* main; /*!< the main state */
	eu_table* internalized; /*!< internalized strings and symbols */
	eu_table* env; /*!< global environment */
};

struct europa_jmplist;

struct europa {
	EU_OBJECT_HEADER
	eu_byte level; /*!< current continuation level */
	eu_byte status; /*!< current execution status */

	eu_global* global; /*!< associated global state */
	struct europa_jmplist* error_jmp; /*!< error jump buf */

	eu_error* err; /*!< last computation error */

	/* i/o ports */
	eu_port* output_port; /*!< current output port */
	eu_port* input_port; /*!< current input port */
	eu_port* error_port; /*!< current error port */

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
#define _euglobal_env(g) ((g)->env)

#define _eu_global(s) ((s)->global)
#define _eu_gc(s) _euglobal_gc(_eu_global(s))
#define _eu_env(s) ((s)->env)
#define _eu_err(s) ((s)->err)
#define _eu_acc(s) (&((s)->acc))
#define _eu_global_env(s) (_euglobal_env(_eu_global(s)))

#define _eu_reset_err(s) (_eu_err(s) = NULL)

europa* eu_new(eu_realloc f, void* ud, eu_cfunc panic, int* err);
int eu_terminate(europa* s);

int eu_set_error(europa* s, int flags, eu_error* nested, void* error_text);
int eu_set_error_nf(europa* s, int flags, eu_error* nested, size_t len, const char* fmt, ...);

int eu_do_string(europa* s, void* text, eu_value* out);

/* TODO: add a define to keep stdio out */
int eu_do_file(europa* s, const char* filename, eu_value* out);

int eu_recover(europa* s, eu_error** err);


eu_uinteger eustate_hash(europa* vec);
int eustate_mark(europa* s, eu_gcmark mark, europa* state);

eu_uinteger euglobal_hash(eu_global* gl);
int euglobal_mark(europa* s, eu_gcmark mark, eu_global* state);

#endif /* __EUROPA_H__ */
