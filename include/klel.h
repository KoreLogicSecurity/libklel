/*-
 ***********************************************************************
 *
 * $Id: klel.h,v 1.78 2012/11/30 18:50:14 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _KLEL_H_INCLUDED
#define _KLEL_H_INCLUDED

#ifdef KLEL_DEBUG
  #include <assert.h>
#endif

#ifdef HAVE_INTTYPES_H
  #include <inttypes.h>
#else
  #ifdef HAVE_STDINT_H
    #include <stdint.h>
  #else
    #include <klel-pstdint.h>
  #endif
#endif

#include <stddef.h>

#ifndef PRIx32
#define PRIx32 PRINTF_INT32_MODIFIER "x"
#endif
#ifndef PRIo32
#define PRIo32 PRINTF_INT32_MODIFIER "o"
#endif
#ifndef PRId32
#define PRId32 PRINTF_INT32_MODIFIER "d"
#endif
#ifndef PRIx64
#define PRIx64 PRINTF_INT64_MODIFIER "x"
#endif
#ifndef PRIo64
#define PRIo64 PRINTF_INT64_MODIFIER "o"
#endif
#ifndef PRId64
#define PRId64 PRINTF_INT64_MODIFIER "d"
#endif

#if __STDC_VERSION__ >= 199901L
  #define KLEL_FLEXIBLE_ARRAY
#else
  #define KLEL_FLEXIBLE_ARRAY 1
#endif

/*-
 ***********************************************************************
 *
 * KLEL_MAX_FUNC_ARGS
 *
 * This constant specifies the maximum number of arguments a function
 * may take. The number 13 is fixed and is determined by the way
 * types are represented.
 *
 ***********************************************************************
 */
#define KLEL_MAX_FUNC_ARGS 13

/*-
 ***********************************************************************
 *
 * KLEL_MAX_NAME
 *
 * This constant represents the maximum length of a named object. This
 * limits the maximum size of a parser node's name, and the maximum
 * length of function/variable names. Literal strings longer than this
 * value are automatically converted to concatenations of shorter
 * strings. This value should only be changed if you expect to have very
 * long variable names.
 *
 ***********************************************************************
 */
#ifndef KLEL_MAX_NAME
#define KLEL_MAX_NAME 255
#endif

/*-
 ***********************************************************************
 *
 * KLEL_DEFAULT_QUOTE_CHAR
 *
 * This constant represents the default character used to quote the
 * members of KlelQuotedChars. In other words, it represents the
 * character that will be placed in front of each quoted character.
 * This can be changed at runtime by calling KlelSetQuoteChar.
 *
 ***********************************************************************
 */
#ifndef KLEL_DEFAULT_QUOTE_CHAR
#define KLEL_DEFAULT_QUOTE_CHAR '\\'
#endif

/*-
 ***********************************************************************
 *
 * KLEL_DEFAULT_QUOTED_CHARS
 *
 * This constant represents the default set of characters that will be
 * quoted when a value is interpolated into a string at runtime with
 * quoting enabled. This can be changed at runtime by assigning a
 * different value to KlelQuotedChars.
 *
 ***********************************************************************
 */
#ifndef KLEL_DEFAULT_QUOTED_CHARS
#define KLEL_DEFAULT_QUOTED_CHARS "'|;&\" \t\r\n><\\"
#endif

/*-
 ***********************************************************************
 *
 * The KLEL_ASSERT macro is reserved for debugging purposes and should
 * not be enabled in production code.
 *
 ***********************************************************************
 */
#ifdef KLEL_DEBUG
#define KLEL_ASSERT(condition) assert(condition)
#else
#define KLEL_ASSERT(condition)
#endif

/*-
 ***********************************************************************
 *
 * The possible types of a KL-EL expression. Every expression has one
 * and only one type. Atomic types must fit within four bits. Function
 * types are constructed using macros to indicate their return type and
 * the type and number of their arguments. All types are packed into
 * 64-bit integers (which are treated as bitfields). The meanings of the
 * bits in these fields is generally unimportant - what matters is that
 * two types that are equivalent always compare to be equal, and that
 * types that are not equivalent do not compare to be equal. The type of
 * an expression can be atomic (boolean, integer, real, or string), or a
 * function type. The type of a function is defined as the function's
 * return type and the number and types of its arguments.
 *
 ***********************************************************************
 */
typedef uint64_t KLEL_EXPR_TYPE;

#define KLEL_TYPE_UNKNOWN             0x0
#define KLEL_TYPE_BOOLEAN             0x1
#define KLEL_TYPE_INT8                0x2
#define KLEL_TYPE_INT16               0x3
#define KLEL_TYPE_INT32               0x4
#define KLEL_TYPE_INT64               0x5
#define KLEL_TYPE_UINT8               0x6
#define KLEL_TYPE_UINT16              0x7
#define KLEL_TYPE_UINT32              0x8
#define KLEL_TYPE_UINT64              0x9
#define KLEL_TYPE_REAL                0xa
#define KLEL_TYPE_STRING              0xb

#define KLEL_ATOMIC_MASK              0xf
#define KLEL_FUNCTION_MASK            ((uint64_t)0x01 << 60)

#define KLEL_SET_FUNCTION_FLAG        ((uint64_t)0x01 << 60)
#define KLEL_SET_ARGUMENT_COUNT(c)    ((uint64_t)(c)  << 56)
#define KLEL_SET_ARGUMENT1(t)         ((uint64_t)(t)  << 52)
#define KLEL_SET_ARGUMENT2(t)         ((uint64_t)(t)  << 48)
#define KLEL_SET_ARGUMENT3(t)         ((uint64_t)(t)  << 44)
#define KLEL_SET_ARGUMENT4(t)         ((uint64_t)(t)  << 40)
#define KLEL_SET_ARGUMENT5(t)         ((uint64_t)(t)  << 36)
#define KLEL_SET_ARGUMENT6(t)         ((uint64_t)(t)  << 32)
#define KLEL_SET_ARGUMENT7(t)         ((uint64_t)(t)  << 28)
#define KLEL_SET_ARGUMENT8(t)         ((uint64_t)(t)  << 24)
#define KLEL_SET_ARGUMENT9(t)         ((uint64_t)(t)  << 20)
#define KLEL_SET_ARGUMENT10(t)        ((uint64_t)(t)  << 16)
#define KLEL_SET_ARGUMENT11(t)        ((uint64_t)(t)  << 12)
#define KLEL_SET_ARGUMENT12(t)        ((uint64_t)(t)  <<  8)
#define KLEL_SET_ARGUMENT13(t)        ((uint64_t)(t)  <<  4)
#define KLEL_SET_RETURN_TYPE(t)       ((uint64_t)(t)  <<  0)

#define KLEL_GET_ARGUMENT_COUNT(t)    (((uint64_t)(t) >> 56) & 0xf)
#define KLEL_GET_ARGUMENT1(t)         (((uint64_t)(t) >> 52) & 0xf)
#define KLEL_GET_ARGUMENT2(t)         (((uint64_t)(t) >> 48) & 0xf)
#define KLEL_GET_ARGUMENT3(t)         (((uint64_t)(t) >> 44) & 0xf)
#define KLEL_GET_ARGUMENT4(t)         (((uint64_t)(t) >> 40) & 0xf)
#define KLEL_GET_ARGUMENT5(t)         (((uint64_t)(t) >> 36) & 0xf)
#define KLEL_GET_ARGUMENT6(t)         (((uint64_t)(t) >> 32) & 0xf)
#define KLEL_GET_ARGUMENT7(t)         (((uint64_t)(t) >> 28) & 0xf)
#define KLEL_GET_ARGUMENT8(t)         (((uint64_t)(t) >> 24) & 0xf)
#define KLEL_GET_ARGUMENT9(t)         (((uint64_t)(t) >> 20) & 0xf)
#define KLEL_GET_ARGUMENT10(t)        (((uint64_t)(t) >> 16) & 0xf)
#define KLEL_GET_ARGUMENT11(t)        (((uint64_t)(t) >> 12) & 0xf)
#define KLEL_GET_ARGUMENT12(t)        (((uint64_t)(t) >>  8) & 0xf)
#define KLEL_GET_ARGUMENT13(t)        (((uint64_t)(t) >>  4) & 0xf)
#define KLEL_GET_RETURN_TYPE(t)       (((uint64_t)(t) >>  0) & 0xf)
#define KLEL_GET_ARGUMENT(t, i)       (((uint64_t)(t) >> ((13 - i) * 4)) & 0xf)
#define KLEL_IS_FUNCTION(x)           ((((uint64_t)(x)) & KLEL_SET_FUNCTION_FLAG) && KLEL_GET_RETURN_TYPE(x) != KLEL_TYPE_UNKNOWN)

#define KLEL_DEFINE_FUNCTION_TYPE(rt, ac, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)   \
  (0x00                                                                                           | \
   KLEL_SET_FUNCTION_FLAG                                                                         | \
   KLEL_SET_ARGUMENT_COUNT(ac)                                                                    | \
   KLEL_SET_ARGUMENT1(a1)                                                                         | \
   KLEL_SET_ARGUMENT2(a2)                                                                         | \
   KLEL_SET_ARGUMENT3(a3)                                                                         | \
   KLEL_SET_ARGUMENT4(a4)                                                                         | \
   KLEL_SET_ARGUMENT5(a5)                                                                         | \
   KLEL_SET_ARGUMENT6(a6)                                                                         | \
   KLEL_SET_ARGUMENT7(a7)                                                                         | \
   KLEL_SET_ARGUMENT8(a8)                                                                         | \
   KLEL_SET_ARGUMENT9(a9)                                                                         | \
   KLEL_SET_ARGUMENT10(a10)                                                                       | \
   KLEL_SET_ARGUMENT11(a11)                                                                       | \
   KLEL_SET_ARGUMENT12(a12)                                                                       | \
   KLEL_SET_ARGUMENT13(a13)                                                                       | \
   KLEL_SET_RETURN_TYPE(rt))

#define KLEL_TYPE_BOOLEAN_FUNCTION0() \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION1(a1) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 1, a1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION2(a1, a2) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 2, a1, a2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION3(a1, a2, a3) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 3, a1, a2, a3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION4(a1, a2, a3, a4) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 4, a1, a2, a3, a4, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION5(a1, a2, a3, a4, a5) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 5, a1, a2, a3, a4, a5, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION6(a1, a2, a3, a4, a5, a6) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 6, a1, a2, a3, a4, a5, a6, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION7(a1, a2, a3, a4, a5, a6, a7) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 7, a1, a2, a3, a4, a5, a6, a7, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION8(a1, a2, a3, a4, a5, a6, a7, a8) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 8, a1, a2, a3, a4, a5, a6, a7, a8, 0, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION9(a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 9, a1, a2, a3, a4, a5, a6, a7, a8, a9, 0, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 10, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, 0, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 11, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, 0, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 12, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, 0)
#define KLEL_TYPE_BOOLEAN_FUNCTION13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_BOOLEAN, 13, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)

#define KLEL_TYPE_INT64_FUNCTION0() \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION1(a1) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 1, a1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION2(a1, a2) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 2, a1, a2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION3(a1, a2, a3) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 3, a1, a2, a3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION4(a1, a2, a3, a4) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 4, a1, a2, a3, a4, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION5(a1, a2, a3, a4, a5) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 5, a1, a2, a3, a4, a5, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION6(a1, a2, a3, a4, a5, a6) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 6, a1, a2, a3, a4, a5, a6, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION7(a1, a2, a3, a4, a5, a6, a7) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 7, a1, a2, a3, a4, a5, a6, a7, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION8(a1, a2, a3, a4, a5, a6, a7, a8) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 8, a1, a2, a3, a4, a5, a6, a7, a8, 0, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION9(a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 9, a1, a2, a3, a4, a5, a6, a7, a8, a9, 0, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 10, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, 0, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 11, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, 0, 0)
#define KLEL_TYPE_INT64_FUNCTION12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 12, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, 0)
#define KLEL_TYPE_INT64_FUNCTION13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_INT64, 13, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)

#define KLEL_TYPE_REAL_FUNCTION0() \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION1(a1) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 1, a1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION2(a1, a2) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 2, a1, a2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION3(a1, a2, a3) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 3, a1, a2, a3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION4(a1, a2, a3, a4) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 4, a1, a2, a3, a4, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION5(a1, a2, a3, a4, a5) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 5, a1, a2, a3, a4, a5, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION6(a1, a2, a3, a4, a5, a6) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 6, a1, a2, a3, a4, a5, a6, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION7(a1, a2, a3, a4, a5, a6, a7) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 7, a1, a2, a3, a4, a5, a6, a7, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION8(a1, a2, a3, a4, a5, a6, a7, a8) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 8, a1, a2, a3, a4, a5, a6, a7, a8, 0, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION9(a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 9, a1, a2, a3, a4, a5, a6, a7, a8, a9, 0, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 10, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, 0, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 11, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, 0, 0)
#define KLEL_TYPE_REAL_FUNCTION12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 12, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, 0)
#define KLEL_TYPE_REAL_FUNCTION13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_REAL, 13, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)

#define KLEL_TYPE_STRING_FUNCTION0() \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION1(a1) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 1, a1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION2(a1, a2) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 2, a1, a2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION3(a1, a2, a3) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 3, a1, a2, a3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION4(a1, a2, a3, a4) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 4, a1, a2, a3, a4, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION5(a1, a2, a3, a4, a5) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 5, a1, a2, a3, a4, a5, 0, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION6(a1, a2, a3, a4, a5, a6) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 6, a1, a2, a3, a4, a5, a6, 0, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION7(a1, a2, a3, a4, a5, a6, a7) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 7, a1, a2, a3, a4, a5, a6, a7, 0, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION8(a1, a2, a3, a4, a5, a6, a7, a8) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 8, a1, a2, a3, a4, a5, a6, a7, a8, 0, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION9(a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 9, a1, a2, a3, a4, a5, a6, a7, a8, a9, 0, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 10, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, 0, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 11, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, 0, 0)
#define KLEL_TYPE_STRING_FUNCTION12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 12, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, 0)
#define KLEL_TYPE_STRING_FUNCTION13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  KLEL_DEFINE_FUNCTION_TYPE(KLEL_TYPE_STRING, 13, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)

/*-
 ***********************************************************************
 *
 * Certain indices of child nodes in certain situations have special
 * meaning. Note that the KLEL_ARGUMENTn_INDEX indices must always be
 * listed first and in numeric order, and that KLEL_MAX_CHILDREN must
 * always be defined as one more than the highest index.
 *
 ***********************************************************************
 */
#define KLEL_ARGUMENT1_INDEX   0  /* Index of argument 1.                  */
#define KLEL_ARGUMENT2_INDEX   1  /* Index of argument 2.                  */
#define KLEL_ARGUMENT3_INDEX   2  /* Index of argument 3.                  */
#define KLEL_ARGUMENT4_INDEX   3  /* Index of argument 4.                  */
#define KLEL_ARGUMENT5_INDEX   4  /* Index of argument 5.                  */
#define KLEL_ARGUMENT6_INDEX   5  /* Index of argument 6.                  */
#define KLEL_ARGUMENT7_INDEX   6  /* Index of argument 7.                  */
#define KLEL_ARGUMENT8_INDEX   7  /* Index of argument 8.                  */
#define KLEL_ARGUMENT9_INDEX   8  /* Index of argument 9.                  */
#define KLEL_ARGUMENT10_INDEX  9  /* Index of argument 10.                 */
#define KLEL_ARGUMENT11_INDEX  10 /* Index of argument 11.                 */
#define KLEL_ARGUMENT12_INDEX  11 /* Index of argument 12.                 */
#define KLEL_ARGUMENT13_INDEX  12 /* Index of argument 13.                 */
#define KLEL_COMMAND_INDEX     13 /* Index of the evaluated command.       */
#define KLEL_DEFINITION_INDEX  14 /* Index of the definition.              */
#define KLEL_DESIGNATOR_INDEX  15 /* Index of the designator.              */
#define KLEL_EXPRESSION_INDEX  16 /* Index of the expression.              */
#define KLEL_FAIL_INDEX        17 /* Index of failure codes.               */
#define KLEL_IFFALSE_INDEX     18 /* Index of negative conditional branch. */
#define KLEL_IFTRUE_INDEX      19 /* Index of positive condtional branch.  */
#define KLEL_INTERPRETER_INDEX 20 /* Index of eval interpreter.            */
#define KLEL_LABEL_INDEX       21 /* Index of the expression label string. */
#define KLEL_OPERAND1_INDEX    22 /* Index of the left operand.            */
#define KLEL_OPERAND2_INDEX    23 /* Index of the right operand.           */
#define KLEL_PASS_INDEX        24 /* Index of success codes.               */
#define KLEL_PREDICATE_INDEX   25 /* Index of guarded command predicate.   */
#define KLEL_MAX_CHILDREN      26 /* The total number of children.         */

/*-
 ***********************************************************************
 *
 * Compiled expressions are represented as trees of KLEL_NODE structures.
 *
 ***********************************************************************
 */
typedef enum _KLEL_NODE_TYPE
{
  KLEL_NODE_ERROR,           /* Errors.           */
  KLEL_NODE_AND,             /* &                 */
  KLEL_NODE_AND_AND,         /* &&                */
  KLEL_NODE_BANG,            /* !                 */
  KLEL_NODE_CALL,            /* Function calls.   */
  KLEL_NODE_CARET,           /* ^                 */
  KLEL_NODE_CLOSE_PAREN,     /* )                 */
  KLEL_NODE_CLOSE_SQUARE,    /* ]                 */
  KLEL_NODE_COLON,           /* :                 */
  KLEL_NODE_COMMA,           /* ,                 */
  KLEL_NODE_CONDITIONAL,     /* Conditionals.     */
  KLEL_NODE_DESIGNATOR,      /* Designators.      */
  KLEL_NODE_DISCARD,         /* Discarded tokens. */
  KLEL_NODE_DOT,             /* .                 */
  KLEL_NODE_EOF,             /* EOF               */
  KLEL_NODE_EQ,              /* =                 */
  KLEL_NODE_EQ_EQ,           /* ==                */
  KLEL_NODE_EVAL,            /* 'eval'            */
  KLEL_NODE_EXPRESSION,      /* Expressions.      */
  KLEL_NODE_FAIL,            /* 'fail'            */
  KLEL_NODE_FRAGMENT,        /* String fragments. */
  KLEL_NODE_GUARDED_COMMAND, /* Guarded commands. */
  KLEL_NODE_GT,              /* >                 */
  KLEL_NODE_GTE,             /* >=                */
  KLEL_NODE_GT_GT,           /* >>                */
  KLEL_NODE_IF,              /* 'if'              */
  KLEL_NODE_IN,              /* 'in'              */
  KLEL_NODE_INTEGER,         /* Integers.         */
  KLEL_NODE_INTERP,          /* ${foo}            */
  KLEL_NODE_LET,             /* 'let'             */
  KLEL_NODE_LIKE,            /* =~                */
  KLEL_NODE_LT,              /* <                 */
  KLEL_NODE_LTE,             /* <=                */
  KLEL_NODE_LT_LT,           /* <<                */
  KLEL_NODE_MINUS,           /* -                 */
  KLEL_NODE_NE,              /* !=                */
  KLEL_NODE_NEGATE,          /* Numeric negation. */
  KLEL_NODE_OPEN_PAREN,      /* (                 */
  KLEL_NODE_OPEN_SQUARE,     /* [                 */
  KLEL_NODE_PASS,            /* 'pass'            */
  KLEL_NODE_PERCENT,         /* %                 */
  KLEL_NODE_PIPE,            /* |                 */
  KLEL_NODE_PIPE_PIPE,       /* ||                */
  KLEL_NODE_PLUS,            /* +                 */
  KLEL_NODE_QUESTION,        /* ?                 */
  KLEL_NODE_QUOTE,           /* "                 */
  KLEL_NODE_QUOTED_INTERP,   /* $(foo)            */
  KLEL_NODE_REAL,            /* Reals.            */
  KLEL_NODE_SLASH,           /* /                 */
  KLEL_NODE_STAR,            /* *                 */
  KLEL_NODE_THEN,            /* 'then'            */
  KLEL_NODE_TILDE,           /* ~                 */
  KLEL_NODE_UNLIKE           /* !~                */
} KLEL_NODE_TYPE;

typedef struct _KLEL_NODE
{
  KLEL_NODE_TYPE    iType;
  int               iReferenceCount;

  int               iClosure;
  size_t            szLength;
  char              acFragment[KLEL_MAX_NAME];
  double            dReal;
  int64_t           llInteger;

  struct _KLEL_NODE *apsChildren[KLEL_MAX_CHILDREN];
} KLEL_NODE;

/*-
 ***********************************************************************
 *
 * The result of an expression's execution is a KLEL_VALUE structure.
 * KLEL_VALUE structures also represent functions and values exported
 * from the host environment into the expression environment.
 *
 ***********************************************************************
 */
typedef struct _KLEL_VALUE
{
  KLEL_EXPR_TYPE     iType;
  int                bBoolean;
  double             dReal;
  struct _KLEL_VALUE *(*fFunction)(struct _KLEL_VALUE **, void *);
  int64_t            llInteger;
  size_t             szLength;
  char               acString[KLEL_FLEXIBLE_ARRAY];
} KLEL_VALUE;

/*-
 ***********************************************************************
 *
 * Expressions are executed relative to a context, which provides a
 * place to stash user-defined data for variable lookups and reported
 * errors. Contexts contain environments, which provide lexically-scoped
 * variable lookups. Environments can be nested. Contexts also provide
 * references to programmer-provided functions that are used to lookup
 * the types and values of variables exported from the runtime.
 * Contexts also provide an error stack. The error stack is where the
 * compiler and runtime push errors as they are encountered.
 *
 ***********************************************************************
 */
typedef KLEL_EXPR_TYPE  (*KLEL_TYPE_CALLBACK)(const char *pcName, void *pvContext);
typedef KLEL_VALUE     *(*KLEL_VALUE_CALLBACK)(const char *pcName, void *pvContext);

typedef struct _KLEL_CLOSURE
{
  KLEL_EXPR_TYPE       iType;
  int                  iIndex;
  char                 acVariable[KLEL_MAX_NAME + 1];
  KLEL_VALUE           *psValue;
  struct _KLEL_CLOSURE *psNext;
} KLEL_CLOSURE;

typedef struct _KLEL_CONTEXT
{
  int                 bIsValid;

  char                *pcName;
  char                *pcInterpreter;
  char                *pcProgram;

  int                 iTokenizationMode;
  const char          *pcInput;

  void                *pvData;

  char                *pcError;
  int                 iSuppressErrors;

  KLEL_TYPE_CALLBACK  pfGetTypeOfVar;
  KLEL_VALUE_CALLBACK pfGetValueOfVar;

  KLEL_NODE           *psExpression;
  KLEL_EXPR_TYPE      iExpressionType;
  int                 aiCodes[256];

  int                 iClosureCount;
  KLEL_CLOSURE        *psEnvironment;
  KLEL_CLOSURE        *psClosures;
} KLEL_CONTEXT;

/*-
 ***********************************************************************
 *
 * Guarded commands are associated with a KLEL_COMMAND structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_COMMAND
{
  char         acInterpreter[KLEL_MAX_NAME + 1];
  char         acProgram[KLEL_MAX_NAME + 1];
  size_t       szArgumentCount;
  char         *ppcArgumentVector[KLEL_MAX_FUNC_ARGS + 1];
  int          aiCodes[256];
} KLEL_COMMAND;

/*-
 ***********************************************************************
 *
 * Compile an expression.
 *
 ***********************************************************************
 */
#define KLEL_MUST_BE_GUARDED_COMMAND                   (1<<0)
#define KLEL_MUST_BE_NAMED                             (1<<1)
#define KLEL_MUST_SPECIFY_RETURN_CODES                 (1<<2)
#define KLEL_MUST_BE_GUARDED_COMMAND_WITH_RETURN_CODES (KLEL_MUST_BE_GUARDED_COMMAND | KLEL_MUST_SPECIFY_RETURN_CODES)

KLEL_CONTEXT *KlelCompile(const char *pcInput, unsigned long ulFlags, const KLEL_TYPE_CALLBACK pfGetTypeOfVar, const KLEL_VALUE_CALLBACK pfGetValueOfVar, void *pvData);

/*-
 ***********************************************************************
 *
 * Execute an expression in its context.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelExecute(KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get information about a compiled expression.
 *
 ***********************************************************************
 */
int            KlelIsValid(const KLEL_CONTEXT *psContext);
int            KlelIsSuccessReturnCode(const KLEL_CONTEXT *psContext, unsigned int uiCode);
uint32_t       KlelGetChecksum(const KLEL_CONTEXT *psContext, unsigned long ulFlags);
char           *KlelGetName(const KLEL_CONTEXT *psContext);
void           *KlelGetPrivateData(const KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelGetTypeOfExpression(const KLEL_CONTEXT *psContext);
int            KlelIsGuardedCommand(const KLEL_CONTEXT *psContext);
KLEL_COMMAND   *KlelGetCommand(KLEL_CONTEXT *psContext);
char           *KlelGetCommandInterpreter(KLEL_CONTEXT *psContext);
char           *KlelGetCommandProgram(KLEL_CONTEXT *psContext);
void           KlelSetPrivateData(KLEL_CONTEXT *psContext, void *pvData);

/*-
 ***********************************************************************
 *
 * Convert an expression into a string.
 *
 ***********************************************************************
 */
#define KLEL_EXPRESSION_ONLY ((unsigned long)0)
#define KLEL_EXPRESSION_PLUS_COMMAND_LINE ((unsigned long)(1<<0))
#define KLEL_EXPRESSION_PLUS_RETURN_CODES ((unsigned long)(1<<1))
#define KLEL_EXPRESSION_PLUS_NAME ((unsigned long)(1<<2))
#define KLEL_EXPRESSION_PLUS_EVERYTHING (KLEL_EXPRESSION_PLUS_COMMAND_LINE | KLEL_EXPRESSION_PLUS_RETURN_CODES | KLEL_EXPRESSION_PLUS_NAME)
char *KlelExpressionToString(const KLEL_CONTEXT *psContext, unsigned long ulFlags);

/*-
 ***********************************************************************
 *
 * Free a compiled expression.
 *
 ***********************************************************************
 */
void KlelFreeContext(KLEL_CONTEXT *psContext);
void KlelFreeCommand(KLEL_COMMAND *psCommand);

/*-
 ***********************************************************************
 *
 * Create a new value.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelCreateValue(KLEL_EXPR_TYPE iType, ...);
#define    KlelCreateBoolean(b)        KlelCreateValue(KLEL_TYPE_BOOLEAN, (int)(b))
#define    KlelCreateReal(r)           KlelCreateValue(KLEL_TYPE_REAL, (double)(r))
#define    KlelCreateInteger(i)        KlelCreateValue(KLEL_TYPE_INT64, (int64_t)(i))
#define    KlelCreateString(l, s)      KlelCreateValue(KLEL_TYPE_STRING, (size_t)(l), (const char *)(s))
#define    KlelCreateFunction(t, n, f) KlelCreateValue((t) | KLEL_SET_FUNCTION_FLAG, n, f)
#define    KlelCreateUnknown()         KlelCreateValue(KLEL_TYPE_UNKNOWN)

/*-
 ***********************************************************************
 *
 * Convert a value to a string.
 *
 ***********************************************************************
 */
void KlelSetQuotedChars(const char *pcChars);
void KlelSetQuoteChar(char cChar);
char *KlelValueToQuotedString(const KLEL_VALUE *psValue, size_t *ulLength);
char *KlelValueToString(const KLEL_VALUE *psValue, size_t *ulLength);

/*-
 ***********************************************************************
 *
 * Free a value.
 *
 ***********************************************************************
 */
void KlelFreeResult(KLEL_VALUE *psResult);

/*-
 ***********************************************************************
 *
 * Work with errors.
 *
 ***********************************************************************
 */
void KlelClearError(KLEL_CONTEXT *psContext);
void KlelReportError(KLEL_CONTEXT *psContext, const char *pcFormat, ...);
void KlelReportMemoryAllocationError(KLEL_CONTEXT *psContext);
const char *KlelGetError(KLEL_CONTEXT *psContext);
const char *KlelGetFirstError(KLEL_CONTEXT *psContext);
const char *KlelGetNextError(KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get a release number or string for the package.
 *
 ***********************************************************************
 */
uint32_t KlelGetReleaseNumber(void);
const char *KlelGetReleaseString(void);

/*-
 ***********************************************************************
 *
 * Get the major, minor, and patch release numbers for the package.
 *
 ***********************************************************************
 */
int KlelGetReleaseMajor(void);
int KlelGetReleaseMinor(void);
int KlelGetReleasePatch(void);

/*-
 ***********************************************************************
 *
 * Get the current, revision, and age version numbers for the library.
 *
 ***********************************************************************
 */
int KlelGetVersionCurrent(void);
int KlelGetVersionRevision(void);
int KlelGetVersionAge(void);

/*-
 ***********************************************************************
 *
 * Backwards Compatibility
 *
 ***********************************************************************
 */
#define KLEL_EXPR_BOOLEAN KLEL_TYPE_BOOLEAN
#define KLEL_EXPR_FLOAT   KLEL_TYPE_REAL
#define KLEL_EXPR_INTEGER KLEL_TYPE_INT64
#define KLEL_EXPR_REAL    KLEL_TYPE_REAL
#define KLEL_EXPR_STRING  KLEL_TYPE_STRING

#endif /* !_KLEL_H_INCLUDED */
