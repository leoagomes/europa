#ifndef __EUROPA_INTEGER_H__
#define __EUROPA_INTEGER_H__

#include <stdint.h>

typedef unsigned char eu_byte; /*!< byte type */
typedef short eu_short; /*!< short (usually 16-bit) type */
typedef unsigned int eu_uint; /*!< unsigned int type */

typedef unsigned int eu_bool; /*!< boolean type */

/* value types for numbers, which can either be floating point Reals or
 * fixed point Integers. */
typedef long long eu_integer;
typedef double eu_real;

/* default boolean true and false values */
#define EU_TRUE !EU_FALSE /*!< boolean true value */
#define EU_FALSE 0x00 /*!< boolean false value */

typedef int eu_result; /*!< c function result type */
enum {
	EU_RESULT_OK = 0,
	EU_RESULT_BAD_ARGUMENT,
	EU_RESULT_NULL_ARGUMENT,
};

#endif /* __EUROPA_INTEGER_H__ */
