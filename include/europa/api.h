#ifndef __EUROPA_API_H__
#define __EUROPA_API_H__

#include <stddef.h>

/** Europa execution state. */
typedef struct europa europa;

/** Allocator function. */
typedef void* (*europa_alloc)(void* ud, void* ptr, size_t size);

/** Bootstraps a new europa instance from a given allocator. */
europa* europa_bootstrap(europa_alloc alloc, void* ud);

#endif
