/*
 *	ISO C99: 7.18 Integer types <stdint.h>
 */

%{
#include <stdint.h>
%}

  
#ifndef  __WORDSIZE64
# if defined(__x86_64__) || (__WORDSIZE == 64)
#  define __WORDSIZE64
# endif
#endif

/* Exact integral types.  */

/* Signed.  */

typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;
#if defined(__WORDSIZE64)
typedef long int		int64_t;
#else
typedef long long int		int64_t;
#endif

/* Unsigned.  */
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;
#if defined(__WORDSIZE64)
typedef unsigned long int	uint64_t;
#else
typedef unsigned long long int	uint64_t;
#endif


/* Small types.  */

/* Signed.  */
typedef signed char		int_least8_t;
typedef short int		int_least16_t;
typedef int			int_least32_t;
#if defined(__WORDSIZE64)
typedef long int		int_least64_t;
#else
typedef long long int		int_least64_t;
#endif

/* Unsigned.  */
typedef unsigned char		uint_least8_t;
typedef unsigned short int	uint_least16_t;
typedef unsigned int		uint_least32_t;
#if defined(__WORDSIZE64)
typedef unsigned long int	uint_least64_t;
#else
typedef unsigned long long int	uint_least64_t;
#endif


/* Fast types.  */

/* Signed.  */
typedef signed char		int_fast8_t;
#if defined(__WORDSIZE64)
typedef long int		int_fast16_t;
typedef long int		int_fast32_t;
typedef long int		int_fast64_t;
#else
typedef int			int_fast16_t;
typedef int			int_fast32_t;
typedef long long int		int_fast64_t;
#endif

/* Unsigned.  */
typedef unsigned char		uint_fast8_t;
#if defined(__WORDSIZE64)
typedef unsigned long int	uint_fast16_t;
typedef unsigned long int	uint_fast32_t;
typedef unsigned long int	uint_fast64_t;
#else
typedef unsigned int		uint_fast16_t;
typedef unsigned int		uint_fast32_t;
typedef unsigned long long int	uint_fast64_t;
#endif


/* Types for `void *' pointers.  */
#if defined(__WORDSIZE64)
typedef long int		intptr_t;
typedef unsigned long int	uintptr_t;
#else
typedef int			intptr_t;
typedef unsigned int		uintptr_t;
#endif


/* Largest integral types.  */
#if defined(__WORDSIZE64)
typedef long int		intmax_t;
typedef unsigned long int	uintmax_t;
#else
typedef long long int		intmax_t;
typedef unsigned long long int	uintmax_t;
#endif


