#ifndef __EUROPA_OBJECTS_H__
#define __EUROPA_OBJECTS_H__

#include "eu_int.h"

#define EU_COMMON_HEAD eu_gcobj* next; eu_byte type; eu_byte color

typedef struct eu_gcobj eu_gcobj;
typedef struct eu_string eu_string;
typedef struct eu_symbol eu_symbol;
typedef struct eu_cell eu_cell;
typedef struct eu_udata eu_udata;


struct eu_gcobj {
	EU_COMMON_HEAD;
};

// allocate the metadata plus size of string
// do some memory alignment
struct eu_string {
	EU_COMMON_HEAD;
	eu_uint size;
};

// same as with string
struct eu_symbol {
	EU_COMMON_HEAD;
	eu_uint size;
	eu_long hash;
};

struct eu_cell {
	EU_COMMON_HEAD;
	eu_gcobj *head, *tail;
};

struct eu_udata {
	EU_COMMON_HEAD;
};

struct eu_table {
	EU_COMMON_HEAD;
};


#define _cast(a,b) (((a))(b))

#define eu_string2gcobj(s) _cast(eu_gcobj*,s)
#define eu_symbol2gcobj(s) _cast(eu_gcobj*,s)
#define eu_cell2gcobj(c) _cast(eu_gcobj*,s)
#define eu_udata2gcobj(u) _cast(eu_gcobj*,s)
#define eu_table2gcobj(t) _cast(eu_gcobj*,s)

#define eu_gcobj2string(o) _cast(eu_string*,o)
#define eu_gcobj2symbol(o) _cast(eu_symbol*,o)
#define eu_gcobj2cell(o) _cast(eu_cell*,o)
#define eu_gcobj2udata(o) _cast(eu_udata*,o)
#define eu_gcobj2table(o) _cast(eu_table*,o)



#endif /* __EUROPA_OBJECTS_H__ */
