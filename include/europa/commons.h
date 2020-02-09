#ifndef __EUROPA_COMMONS_H__
#define __EUROPA_COMMONS_H__

#ifndef NULL
#define NULL ((void*)0)
#endif /* NULL */

#define cast(t,p) ((t)(p))

#define _eu_checkreturn(exp) do {\
		eu_result res;\
		if ((res = (exp)))\
			return res;\
	} while (0)

#endif /* __EUROPA_COMMONS_H__ */
