#ifndef __EUROPA_CHARACTER_H__
#define __EUROPA_CHARACTER_H__

#include "europa.h"
#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

/* helper macros */
#define _eu_makechar(vptr, c) \
	do { \
		(vptr)->type = EU_TYPE_CHARACTER; \
		(vptr)->value.character = (int)c; \
	} while(0)

#endif
