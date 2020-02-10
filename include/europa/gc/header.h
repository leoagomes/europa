#ifndef __EUROPA_GC_HEADER_H__
#define __EUROPA_GC_HEADER_H__

#include "europa/int.h"
#include "europa/common.h"

struct europa_object;

#define EU_OBJECT_GC_HEADER \
	struct europa_object *_previous; /*!< previous heap object list item */\
	struct europa_object *_next; /*!< next heap object list item */\
	unsigned int _reference_count; /*!< number of references to the object */\
	unsigned char _color; /*!< m&s object color */\
	unsigned char _type; /*!< object type */

#endif
