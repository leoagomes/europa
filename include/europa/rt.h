#ifndef __EUROPA_RUNTIME_H__
#define __EUROPA_RUNTIME_H__

#include "europa/europa.h"
#include "europa/object.h"
#include "europa/gc.h"
#include "europa/table.h"
#include "europa/pair.h"
#include "europa/int.h"
#include "europa/common.h"
#include "europa/port.h"

/* type definitions */
typedef int (*eu_pfunc)(europa* s, void* ud);
typedef struct europa_closure eu_closure;
typedef struct europa_continuation eu_continuation;
typedef struct europa_proto eu_proto;
typedef unsigned int eu_instruction;

/** Function prototype. */
struct europa_proto {
	EU_OBJECT_HEADER

	eu_value formals; /*!< formal parameters */

	eu_value* constants; /*!< constants used in function */
	int constantc; /*!< number of constants */
	int constants_size; /*!< size of the constant array */

	eu_value source; /*!< given source */

	eu_proto** subprotos; /*!< prototype for functions defined in this function */
	int subprotoc; /*!< number of sub prototypes */
	int subprotos_size; /*!< size of prototype buffer */

	eu_instruction* code; /*!< prototype code */
	int code_length; /*!< code length */
	int code_size; /*!< code buffer size */
};

/** Closure structure. */
struct europa_closure {
	EU_OBJECT_HEADER
	eu_byte own_env; /*!< whether the closure should have its own environment */

	eu_table* env; /*!< closure creation environment */

	eu_proto* proto; /*!< europa function prototype */
	eu_cfunc cf; /*!< C function closure */
};

/** Continuation structure. */
struct europa_continuation {
	EU_OBJECT_HEADER

	eu_continuation* previous; /*!< previous call frame */

	eu_table* env; /*!< call frame environment */
	eu_value rib; /*!< frame value rib */
	eu_value* rib_lastpos; /*!< last rib slot position */

	eu_closure* cl; /*!< closure in execution */
	unsigned int pc; /*!< saved program counter */
};

/* vm instruction set related definitions */
enum {
	EU_OP_NOP,
	EU_OP_REFER,
	EU_OP_CONST,
	EU_OP_CLOSE,
	EU_OP_TEST,
	EU_OP_JUMP,
	EU_OP_ASSIGN,
	EU_OP_ARGUMENT,
	EU_OP_CONTI,
	EU_OP_APPLY,
	EU_OP_RETURN,
	EU_OP_FRAME,
	EU_OP_DEFINE,
	EU_OP_HALT,
};

enum {
	EU_CLSTATUS_WAITING,
	EU_CLSTATUS_RUNNING,
	EU_CLSTATUS_STOPPED,
	EU_CLSTATUS_FINISHED,
};

/* warning: currently assumes 32-bit */
/* adapting the following values should be enough, though */
#define OPCMASK 0xFF
#define OPCSHIFT 24
#define VALMASK 0xFFFFFF
#define OFFBIAS (0xFFFFFF >> 1)

/* prototype structure functions and macros */
#define _euproto_to_obj(s) cast(eu_object*, s)
#define _euobj_to_proto(o) cast(eu_proto*, o)
#define _euvalue_to_proto(v) _euobj_to_proto((v)->value.object)
#define _eu_makeproto(vptr, s) do {\
		(vptr)->type = EU_TYPE_CLOSURE | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euproto_to_obj(s);\
	} while (0)
#define _euproto_formals(p) (&((p)->formals))

eu_proto* euproto_new(europa* s, eu_value* formals, int constants_size,
	eu_value* source, int subprotos_size, int code_size);
int euproto_mark(europa* s, eu_gcmark mark, eu_proto* p);
int euproto_destroy(europa* s, eu_proto* p);
eu_integer euproto_hash(eu_proto* p);
eu_integer euproto_append_instruction(europa* s, eu_proto* proto,
	eu_instruction inst);
eu_integer euproto_add_constant(europa* s, eu_proto* proto, eu_value* constant,
	int* index);
eu_integer euproto_add_subproto(europa* s, eu_proto* proto, eu_proto* subproto,
	int* index);

/* closure structure macros and functions */
#define _euclosure_to_obj(s) cast(eu_object*, s)
#define _euobj_to_closure(o) cast(eu_closure*, o)
#define _euvalue_to_closure(v) _euobj_to_closure((v)->value.object)
#define _eu_makeclosure(vptr, s) do {\
		(vptr)->type = EU_TYPE_CLOSURE | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euclosure_to_obj(s);\
	} while (0)

eu_closure* eucl_new(europa* s, eu_cfunc cf, eu_proto* proto, eu_table* env);

int eucl_mark(europa* s, eu_gcmark mark, eu_closure* cl);
int eucl_destroy(europa* s, eu_closure* cl);
eu_integer eucl_hash(eu_closure* cl);

/* continuation structure macros and functions */
#define _eucont_to_obj(s) cast(eu_object*, s)
#define _euobj_to_cont(o) cast(eu_continuation*, o)
#define _euvalue_to_cont(v) _euobj_to_cont((v)->value.object)
#define _eu_makecont(vptr, s) do {\
		(vptr)->type = EU_TYPE_CONTINUATION | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eucont_to_obj(s);\
	} while (0)

eu_continuation* eucont_new(europa* s, eu_continuation* previous,
	eu_table* env, eu_value* rib, eu_value* rib_lastpos, eu_closure* cl,
	unsigned int pc);

int eucont_mark(europa* s, eu_gcmark mark, eu_continuation* cl);
int eucont_destroy(europa* s, eu_continuation* cl);
eu_integer eucont_hash(eu_continuation* cl);

/* code generation related macros and functions */
int eucode_compile(europa* s, eu_value* v, eu_value* chunk);

/* virtual machine related functions and macros */
int euvm_doclosure(europa* s, eu_closure* cl, eu_value* arguments,
	eu_value* out);
int euvm_initialize_state(europa* s);
int euvm_apply(europa* s, eu_value* v, eu_value* args, eu_value* out);
int euvm_disassemble(europa* s, eu_port* port, eu_value* v);

/* run time macros and functions */
int eurt_evaluate(europa* s, eu_value* value,  eu_value* out);

/* library */
int euapi_register_controls(europa* s);

int euapi_procedureQ(europa* s);
int euapi_apply(europa* s);
int euapi_map(europa* s);
int euapi_for_each(europa* s);

int euapi_disassemble(europa* s);

#endif
