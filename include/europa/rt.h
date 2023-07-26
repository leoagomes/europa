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

#include "europa/types.h"

/* type definitions */

/** Function prototype. */
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
#define _euproto_to_obj(s) cast(struct europa_object*, s)
#define _euobj_to_proto(o) cast(struct europa_prototype*, o)
#define _euvalue_to_proto(v) _euobj_to_proto((v)->value.object)
#define _eu_makeproto(vptr, s) do {\
		(vptr)->type = EU_TYPE_CLOSURE | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euproto_to_obj(s);\
	} while (0)
#define _euproto_formals(p) (&((p)->formals))

struct europa_prototype* euproto_new(europa* s, struct europa_value* formals, int constants_size,
	struct europa_value* source, int subprotos_size, int code_size);
int euproto_mark(europa* s, europa_gc_mark mark, struct europa_prototype* p);
int euproto_destroy(europa* s, struct europa_prototype* p);
eu_integer euproto_hash(struct europa_prototype* p);
eu_integer euproto_append_instruction(europa* s, struct europa_prototype* proto,
	eu_instruction inst);
eu_integer euproto_add_constant(europa* s, struct europa_prototype* proto, struct europa_value* constant,
	int* index);
eu_integer euproto_add_subproto(europa* s, struct europa_prototype* proto, struct europa_prototype* subproto,
	int* index);

/* closure structure macros and functions */
#define _euclosure_to_obj(s) cast(struct europa_object*, s)
#define _euobj_to_closure(o) cast(struct europa_closure*, o)
#define _euvalue_to_closure(v) _euobj_to_closure((v)->value.object)
#define _eu_makeclosure(vptr, s) do {\
		(vptr)->type = EU_TYPE_CLOSURE | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euclosure_to_obj(s);\
	} while (0)

struct europa_closure* eucl_new(europa* s, europa_c_callback cf, struct europa_prototype* proto, struct europa_table* env);

int eucl_mark(europa* s, europa_gc_mark mark, struct europa_closure* cl);
int eucl_destroy(europa* s, struct europa_closure* cl);
eu_integer eucl_hash(struct europa_closure* cl);

/* continuation structure macros and functions */
#define _eucont_to_obj(s) cast(struct europa_object*, s)
#define _euobj_to_cont(o) cast(struct europa_continuation*, o)
#define _euvalue_to_cont(v) _euobj_to_cont((v)->value.object)
#define _eu_makecont(vptr, s) do {\
		(vptr)->type = EU_TYPE_CONTINUATION | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eucont_to_obj(s);\
	} while (0)

struct europa_continuation* eucont_new(europa* s, struct europa_continuation* previous,
	struct europa_table* env, struct europa_value* rib, struct europa_value* rib_lastpos, struct europa_closure* cl,
	unsigned int pc);

int eucont_mark(europa* s, europa_gc_mark mark, struct europa_continuation* cl);
int eucont_destroy(europa* s, struct europa_continuation* cl);
eu_integer eucont_hash(struct europa_continuation* cl);

/* code generation related macros and functions */
int eucode_compile(europa* s, struct europa_value* v, struct europa_value* chunk);

/* virtual machine related functions and macros */
int euvm_doclosure(europa* s, struct europa_closure* cl, struct europa_value* arguments,
	struct europa_value* out);
int euvm_initialize_state(europa* s);
int euvm_apply(europa* s, struct europa_value* v, struct europa_value* args, struct europa_value* out);
int euvm_disassemble(europa* s, struct europa_port* port, struct europa_value* v);

/* run time macros and functions */
int eurt_evaluate(europa* s, struct europa_value* value,  struct europa_value* out);

/* library */
int euapi_register_controls(europa* s);

int euapi_procedureQ(europa* s);
int euapi_apply(europa* s);
int euapi_map(europa* s);
int euapi_for_each(europa* s);

int euapi_disassemble(europa* s);

#endif
