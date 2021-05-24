#ifndef __EUROPA_RUNTIME__
#define __EUROPA_RUNTIME__

struct europa;

typedef int (*europa_protected_fn)(struct europa*, void*);

int europa_runtime_run_protected(struct europa*, europa_protected_fn, void*);

int europa_throw_message(struct europa*, char* format, ...);

#endif /* __EUROPA_RUNTIME__ */
