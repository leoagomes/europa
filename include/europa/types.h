#ifndef __EUROPA_TYPES_H__
#define __EUROPA_TYPES_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "europa/common.h"
#include "europa/int.h"

/** The realloc-like function provided to the garbage collector. */
typedef void* (*europa_realloc)(void*, void*, size_t);
typedef int (*europa_c_callback)(europa* s);
typedef int (*europa_protected_function)(europa* s, void* ud);
typedef struct europa europa;

/* forward struct definitions */
struct europa_object;
struct europa_value;
struct europa_frame;
struct europa_pair;
struct europa_error;
struct europa_port;
struct europa_closure;
struct europa_continuation;
struct europa_table;

struct europa_global;
struct europa_jump_list;
struct europa;

/* internal value representation. */
/** enum representing the possible types for tagged value */
enum {
    EU_TYPE_NULL = 0,
    EU_TYPE_BOOLEAN,
    EU_TYPE_NUMBER,
    EU_TYPE_CHARACTER,
    EU_TYPE_EOF,

    EU_TYPE_SYMBOL,
    EU_TYPE_STRING,
    EU_TYPE_ERROR,

    EU_TYPE_PAIR,
    EU_TYPE_VECTOR,
    EU_TYPE_BYTEVECTOR,
    EU_TYPE_TABLE,
    EU_TYPE_PORT,

    EU_TYPE_CLOSURE,
    EU_TYPE_CONTINUATION,
    EU_TYPE_PROTO, /* function prototype */

    EU_TYPE_STATE,
    EU_TYPE_GLOBAL,
    EU_TYPE_CPOINTER,
    EU_TYPE_USERDATA,
    EU_TYPE_LAST
};

extern const char* eu_type_names[];

/** internal value representation structure */
struct europa_value {
    union europa_values {
        eu_integer i; /*!< (fixnum) integer number value */
        eu_real r; /*!< (floating) real number value */
        int boolean; /*!< booleans */
        int character; /*!< characters */

        void* p; /*!< (unmanaged) c pointer */

        struct europa_object* object;
        struct europa_symbol* symbol;
        struct europa_string* string;
        // struct europa_error* error;
        struct europa_pair* pair;
        struct europa_vector* vector;
        struct europa_bytevector* bytevector;
        struct europa_table* table;
        struct europa_port* port;

        struct europa_closure* closure;
        struct europa_continuation* continuation;
        struct europa_prototype* proto;

        struct europa_state* state;
        struct europa_global* global;
    } value;

    eu_byte type; /*!< the value's type */
};

/* garbage collected objects */
#define EUROPA_OBJECT_HEADER \
    struct europa_object *_previous, *_next; /*!< previous heap object list item */\
    unsigned int _tag /*!< object tag */

/** Garbage Collected object abstraction. */
struct europa_object {
    EUROPA_OBJECT_HEADER;
};

/** Symbol */
struct europa_symbol {
    EUROPA_OBJECT_HEADER;

    eu_integer hash;
    size_t length;
    char text[0];
};

/** String */
struct europa_string {
    EUROPA_OBJECT_HEADER;

    eu_integer length;
    eu_integer hash;
    char text[0];
};

/** Error */
struct europa_error {
    EUROPA_OBJECT_HEADER;

    struct europa_error* nested_error;
    struct europa_string* message;
    int flags;
};

/** Pair. */
struct europa_pair {
    EUROPA_OBJECT_HEADER;

    struct europa_value head, tail;
};

/** Vector. */
struct europa_vector {
    EUROPA_OBJECT_HEADER;

    eu_integer length;
    struct europa_value data[0];
};

/** Bytevector. */
struct europa_bytevector {
    EUROPA_OBJECT_HEADER;

    eu_integer length;
    eu_byte data[0];
};

/** Table. */
struct europa_table {
    EUROPA_OBJECT_HEADER;

    eu_byte lsize; /*!< log2 of the table's size */
    int count; /*!< the number of elements in the table */

    struct europa_table_node *nodes, *last_free;
    struct europa_table* index; /*!< the table's index */
};

struct europa_global {
    EUROPA_OBJECT_HEADER;

    struct europa* main; /*!< the main state */
    struct europa_table* internalized; /*!< internalized strings and symbols */
    struct europa_table* env; /*!< global environment */
    europa_c_callback panic;

    struct europa_gc {
        void* realloc_ud;
        europa_realloc realloc;

        struct europa_object root,      /* root object list */
                             thisgen,   /* this generation of objects */
                             *last_obj; /* last created object */
    } gc;
};

typedef int (*europa_gc_mark)(europa* s, struct europa_object* obj);

struct europa {
    EUROPA_OBJECT_HEADER;

    eu_byte level; /*!< current continuation level */
    eu_byte status; /*!< current execution status */

    struct europa_global* global;
    struct europa_jump_list* jump_list;

    struct europa_error* last_error;

    /* i/o ports */
    struct europa_port* output_port;
    struct europa_port* input_port;
    struct europa_port* error_port;

    /* execution state */
    unsigned int program_counter;
    struct europa_closure* current_closure;
    struct europa_value accumulator;
    struct europa_table* environment;
    struct europa_value rib;
    struct europa_value* rib_last_position;
    struct europa_continuation* previous;
};

typedef unsigned int eu_instruction;
struct europa_prototype {
    EUROPA_OBJECT_HEADER;

    struct europa_value formals; /*!< formal parameters */

    struct europa_value* constants; /*!< constants used in function */
    int constantc; /*!< number of constants */
    int constants_size; /*!< size of the constant array */

    struct europa_value source; /*!< given source */

    struct europa_prototype** subprotos; /*!< prototype for functions defined in this function */
    int subprotoc; /*!< number of sub prototypes */
    int subprotos_size; /*!< size of prototype buffer */

    eu_instruction* code; /*!< prototype code */
    int code_length; /*!< code length */
    int code_size; /*!< code buffer size */
};

/** Closure structure. */
struct europa_closure {
    EUROPA_OBJECT_HEADER;

    eu_byte own_env; /*!< whether the closure should have its own environment */

    struct europa_table* env; /*!< closure creation environment */

    struct europa_prototype* proto; /*!< europa function prototype */
    europa_c_callback cf; /*!< C function closure */
};

/** Continuation structure. */
struct europa_continuation {
    EUROPA_OBJECT_HEADER;

    struct europa_continuation* previous; /*!< previous call frame */

    struct europa_table* env; /*!< call frame environment */
    struct europa_value rib; /*!< frame value rib */
    struct europa_value* rib_lastpos; /*!< last rib slot position */

    struct europa_closure* cl; /*!< closure in execution */
    unsigned int pc; /*!< saved program counter */
};

struct europa_table_node {
    /* because of an ugly hack below, value needs to be the first field
     * (this forces &tn->value and tn to the same value and the conversion cast
     * works)
     */
    struct europa_value value;
    struct europa_value key;
    int next;
};

struct europa_table {
    EUROPA_OBJECT_HEADER;

    eu_byte lsize; /*!< log2 of the table's size */
    int count; /*!< the number of elements in the table */
    struct europa_table_node *nodes, *last_free;

    struct europa_table* index; /*!< the table's index */
};

#define EU_PORT_COMMON_HEADER \
    EUROPA_OBJECT_HEADER; \
    eu_byte flags, type

struct europa_port {
    EU_PORT_COMMON_HEADER;
};

struct europa_vport {
    EU_PORT_COMMON_HEADER;
};

struct europa_file_port {
    EU_PORT_COMMON_HEADER;

    FILE* file;
};

struct europa_mport {
    EU_PORT_COMMON_HEADER;

    eu_integer rpos, wpos;
    eu_byte* next;

    size_t size;
    eu_byte* mem;
};

/* helper macros */
#define euvalue_get(v, t) ((v)->value.t)

#endif
