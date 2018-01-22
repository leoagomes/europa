#ifndef __EUROPA_FILE_PORT_H__
#define __EUROPA_FILE_PORT_H__

#include "eu_port.h"

#include <stdio.h>

typedef struct eu_fileport eu_fileport;

struct eu_fileport {
	EU_COMMON_HEAD;
	EU_PORT_COMMON_HEAD;

	FILE* f;
};

#define eu_gcobj2fileport(obj) cast(eu_fileport*, (obj))
#define eu_port2fileport(p) cast(eu_fileport*, (p))

#define eu_fileport2gcobj(fp) cast(eu_gcobj*, (fp))
#define eu_fileport2port(fp) cast(eu_port*, (fp))

eu_fileport* eufp_new(europa* s, const char* mode);
void eufp_destroy(europa_gc* gc, eu_fileport* fp);

#endif /* __EUROPA_FILE_PORT_H__ */