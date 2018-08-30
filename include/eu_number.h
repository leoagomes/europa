#ifndef __EUROPA_NUMBER_H__
#define __EUROPA_NUMBER_H__

#include "europa.h"
#include "eu_object.h"
#include "eu_int.h"
#include "eu_commons.h"

#define EU_NUMBER_REAL EU_TYPEFLAG_EXTRA

#define _eu_makeint(vptr, num) \
	do { \
		(vptr)->type = EU_TYPE_NUMBER; \
		(vptr)->value.i = num; \
	} while(0)

#define _eu_makereal(vptr, num) \
	do { \
		(vptr)->type = EU_TYPE_NUMBER | EU_NUMBER_REAL; \
		(vptr)->value.r = num; \
	} while(0)

#endif