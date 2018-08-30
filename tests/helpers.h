#ifndef __AUX_H__
#define __AUX_H__

#include "europa.h"

void* rlike(void* ud, void* ptr, unsigned long long size);
europa* bootstrap_default_instance(void);
void terminate_default_instance(europa* s);

#endif