#ifndef __EUROPA_H__
#define __EUROPA_H__

#include "eu_commons.h"
#include "eu_int.h"

#include "eu_gc.h"
#include "eu_object.h"
#include "eu_table.h"

typedef struct europa_global eu_global;
typedef struct europa europa;
typedef int (*eu_cfunc)(europa* s);

typedef struct europa_table eu_table;
struct europa_global {
	eu_gc gc; /*!< global state GC */
	eu_cfunc panic; /*!< global panic function */
	europa* main; /*!< the main state */
	eu_table* type_indexes[EU_TYPE_LAST]; /*!< index tables for primitive types */
	eu_table* internalized; /*!< internalized strings and symbols */
};

struct europa_jmplist;

struct europa {
	eu_global* global; /*!< associated global state */
	struct europa_jmplist* error_jmp; /*!< error jump buf */
};

#define _euglobal_gc(g) (&((g)->gc))
#define _euglobal_main(g) ((g)->main)
#define _euglobal_panic(g) ((g)->panic)

#define _eu_global(s) ((s)->global)
#define _eu_gc(s) _euglobal_gc(_eu_global(s))

eu_result euglobal_init(eu_global* g, eu_realloc f, void* ud, eu_cfunc panic);
eu_result europa_init(europa* s);

europa* europa_new(eu_realloc f, void* ud, eu_cfunc panic, eu_result* err);

#endif /* __EUROPA_H__ */
