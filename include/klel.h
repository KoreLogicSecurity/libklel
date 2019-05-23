/*-
 ***********************************************************************
 *
 * $Id: klel.h,v 1.56 2012/04/25 18:35:31 rking Exp $
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
#endif

#ifdef HAVE_STDINT_H
  #include <stdint.h>
#else
  #include "pstdint.h"
  #define PRIx32 PRINTF_INT32_MODIFIER "x"
  #define PRIo32 PRINTF_INT32_MODIFIER "o"
  #define PRId32 PRINTF_INT32_MODIFIER "d"
  #define PRIx64 PRINTF_INT64_MODIFIER "x"
  #define PRIo64 PRINTF_INT64_MODIFIER "o"
  #define PRId64 PRINTF_INT64_MODIFIER "d"
#endif

#if __STDC_VERSION__ >= 199901L
  #define FLEXIBLE_ARRAY
#else
  #define FLEXIBLE_ARRAY 1
#endif

/*-
 ***********************************************************************
 *
 * KLEL_COMPILE_ERROR_FORMAT
 *
 * This constant represents the default format string for compile-time
 * errors.
 *
 ***********************************************************************
 */
#ifndef KLEL_COMPILE_ERROR_FORMAT
#define KLEL_COMPILE_ERROR_FORMAT "%s near '%s'"
#endif

/*-
 ***********************************************************************
 *
 * KLEL_MAX_CHILDREN
 *
 * This constant affects the maximum number of children a single node
 * in the abstract syntax tree may have. This affects the number of
 * arguments a function may take, and how many interpolations and hex
 * escapes can be in a string constant.
 *
 ***********************************************************************
 */
#ifndef KLEL_MAX_CHILDREN
#define KLEL_MAX_CHILDREN 64
#endif
#if KLEL_MAX_CHILDREN < 5
#error "ASSERT(KLEL_MAX_CHILDREN >= 5)"
#endif

/*-
 ***********************************************************************
 *
 * KLEL_MAX_ERROR
 *
 * This constant represents the maximum length in characters (C
 * characters, not code points) of an error message from the parser or
 * runtime. This limit should only need to be changed if a
 * user-provided function produces longer error messages.
 *
 ***********************************************************************
 */
#ifndef KLEL_MAX_ERROR
#define KLEL_MAX_ERROR 127
#endif
#if KLEL_MAX_ERROR < 127
#error "ASSERT(KLEL_MAX_ERROR >= 127)"
#endif

/*-
 ***********************************************************************
 *
 * KLEL_MAX_FUNC_ARGS
 *
 * This constant specifies the maximum number of arguments a function
 * may take. The standard library does not have any functions of more
 * than a few arguments, but user-provided functions may take more.
 * This number must be less than KLEL_MAX_CHILDREN.
 *
 ***********************************************************************
 */
#ifndef KLEL_MAX_FUNC_ARGS
#define KLEL_MAX_FUNC_ARGS 63
#endif
#if KLEL_MAX_FUNC_ARGS < 3 || KLEL_MAX_FUNC_ARGS >= KLEL_MAX_CHILDREN
#error "ASSERT(KLEL_MAX_FUNC_ARGS >= 3 && KLEL_MAX_FUNC_ARGS < KLEL_MAX_CHILDREN)"
#endif

/*-
 ***********************************************************************
 *
 * KLEL_MAX_NAME
 *
 * This constant represents the maximum length of a named object. This
 * limits the maximum size of a parser node's name, and the maximum
 * length of function/variable names, as well as the length of string
 * representation of functions.
 *
 ***********************************************************************
 */
#ifndef KLEL_MAX_NAME
#define KLEL_MAX_NAME 63
#endif
#if KLEL_MAX_NAME < 63
#error "ASSERT(KLEL_MAX_NAME >= 63)"
#endif

/*-
 ***********************************************************************
 *
 * KLEL_QUOTE_CHAR
 *
 * This constant represents the default character used to quote the
 * members of KlelQuotedChars. In other words, it represents the
 * character that will be placed in front of each quoted character.
 * This can be changed at runtime by calling KlelSetQuoteChar.
 *
 ***********************************************************************
 */
#ifndef KLEL_QUOTE_CHAR
#define KLEL_QUOTE_CHAR '\\'
#endif

/*-
 ***********************************************************************
 *
 * KLEL_QUOTED_CHARS
 *
 * This constant represents the default set of characters that will be
 * quoted when a value is interpolated into a string at runtime with
 * quoting enabled. This can be changed at runtime by assigning a
 * different value to KlelQuotedChars.
 *
 ***********************************************************************
 */
#ifndef KLEL_QUOTED_CHARS
#define KLEL_QUOTED_CHARS "'|;&\" \t\r\n><"
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
 * The possible types of a Kl-El expression. Every expression has one
 * and only one type.
 *
 ***********************************************************************
 */
typedef enum _KLEL_EXPR_TYPE
{
  KLEL_EXPR_UNKNOWN,             /* Unknown type. This is always an error. */
  KLEL_EXPR_BOOLEAN,             /* Boolean type.                          */
  KLEL_EXPR_FLOAT,               /* C double type.                         */
  KLEL_EXPR_FUNCTION_BOOLEAN,    /* Function of n arguments to Boolean.    */
  KLEL_EXPR_FUNCTION_FLOAT,      /* Function of n arguments to C double.   */
  KLEL_EXPR_FUNCTION_INTEGER,    /* Function of n arguments to C integer.  */
  KLEL_EXPR_FUNCTION_STRING,     /* Function of n arguments to string.     */
  KLEL_EXPR_INTEGER,             /* int64_t integer.                       */
  KLEL_EXPR_STRING               /* String, possibly with embedded NULs.   */
} KLEL_EXPR_TYPE;

/*-
 ***********************************************************************
 *
 * Expressions can be executed relative to a context, which provides a
 * place to stash user-defined data for variable lookups and reported
 * errors.
 *
 ***********************************************************************
 */
typedef struct _KLEL_ERROR
{
  struct _KLEL_ERROR *psNext;
  char                acError[FLEXIBLE_ARRAY];
} KLEL_ERROR;

typedef struct _KLEL_CONTEXT
{
  KLEL_ERROR  *psErrorList;
  KLEL_ERROR  *psNextError;
  void        *pvData;
} KLEL_CONTEXT;

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
  KLEL_EXPR_TYPE      iType;
  int                 bBoolean;
  double              dFloat;
  struct _KLEL_VALUE *(*fFunction)(struct _KLEL_VALUE **, void *);
  int64_t             llInteger;
  size_t              szLength;
  char                acError[KLEL_MAX_ERROR + 1];
  char                acString[FLEXIBLE_ARRAY];
} KLEL_VALUE;

/*-
 ***********************************************************************
 *
 * Compiled expressions are represented as trees of KLEL_NODE structures.
 *
 ***********************************************************************
 */
typedef enum _KLEL_NODE_TYPE
{
  KLEL_NODE_INVALID,
  KLEL_NODE_TERMINAL,
  KLEL_NODE_NONTERMINAL
} KLEL_NODE_TYPE;

typedef struct _KLEL_NODE
{
  KLEL_NODE_TYPE      iType;
  char                acName[KLEL_MAX_NAME + 1];
  struct _KLEL_NODE  *apsChildren[KLEL_MAX_CHILDREN];
  const char         *pcInput;
  size_t              szLength;
  const char         *pcRemainder;
  char                acError[KLEL_MAX_ERROR + 1];
  char                acNear[KLEL_MAX_ERROR + 1];
  char                cChar;
  double              dFloat;
  int64_t             llInteger;
  char                acVariable[KLEL_MAX_NAME + 1];
  char                acString[FLEXIBLE_ARRAY];
} KLEL_NODE;

/*-
 ***********************************************************************
 *
 * Certain indices of child nodes in certain situations have special
 * meaning.
 *
 ***********************************************************************
 */
#define KLEL_NAME_CHILD_INDEX       0 /* First child of a root node.   */
#define KLEL_EXPRESSION_CHILD_INDEX 1 /* Second child of a root node.  */
#define KLEL_GUARD_CHILD_INDEX      0 /* First child of a guard node.  */
#define KLEL_COMMAND_CHILD_INDEX    1 /* Second child of a guard node. */
#define KLEL_SUCCESS_CHILD_INDEX    2 /* Third child of a guard node.  */
#define KLEL_FAILURE_CHILD_INDEX    3 /* Fourth child of a guard node. */

/*-
 ***********************************************************************
 *
 * Guarded commands are associated with a KLEL_COMMAND structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_COMMAND
{
  char         *pcInterpreter;
  char         *pcProgram;
  size_t       szArgumentCount;
  char         *ppcArgumentVector[KLEL_MAX_FUNC_ARGS + 1];
} KLEL_COMMAND;

/*-
 ***********************************************************************
 *
 * User-provided callbacks. These pointers point to functions used by
 * the runtime to look up the types or values of variables, or to report
 * errors.
 *
 ***********************************************************************
 */
extern KLEL_EXPR_TYPE *(*KlelGetFuncDesc)(const char *pcName, KLEL_CONTEXT *psContext);
extern KLEL_EXPR_TYPE (*KlelGetTypeOfVar)(const char *pcName, KLEL_CONTEXT *psContext);
extern KLEL_VALUE *(*KlelGetValueOfVar)(const char *pcName, KLEL_CONTEXT *psContext);
extern void (*KlelReportError)(KLEL_CONTEXT *psContext, const char *pcFormat, ...);

/*-
 ***********************************************************************
 *
 * Clear the error stack.
 *
 ***********************************************************************
 */
void KlelClearErrors(KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Compile an expression.
 *
 ***********************************************************************
 */
#define KLEL_MUST_BE_GUARDED_COMMAND   (1<<0)
#define KLEL_MUST_BE_NAMED             (1<<1)
#define KLEL_MUST_SPECIFY_COMMAND_ARGS (1<<2)
#define KLEL_MUST_SPECIFY_RETURN_CODES (1<<3)
KLEL_NODE *KlelCompile(const char *pcInput, unsigned long ulFlags, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Create a new context.
 *
 ***********************************************************************
 */
KLEL_CONTEXT *KlelCreateContext(void *pvData);

/*-
 ***********************************************************************
 *
 * Create a new value.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelCreateValue(KLEL_EXPR_TYPE iType, ...);
#define KlelCreateBoolean(f) KlelCreateValue(KLEL_EXPR_BOOLEAN, (int)(f))
#define KlelCreateFloat(f) KlelCreateValue(KLEL_EXPR_FLOAT, (double)(f))
#define KlelCreateInteger(i) KlelCreateValue(KLEL_EXPR_INTEGER, (int64_t)(i))
#define KlelCreateString(l, s) KlelCreateValue(KLEL_EXPR_STRING, (size_t)(l), (const char *)(s))
#define KlelCreateBooleanFunc(name, func) \
  KlelCreateValue(KLEL_EXPR_FUNCTION_BOOLEAN, (const char *)name, (KLEL_VALUE *(*)(KLEL_VALUE **, void *))(func))
#define KlelCreateFloatFunc(name, func) \
  KlelCreateValue(KLEL_EXPR_FUNCTION_FLOAT, (const char *)name, (KLEL_VALUE *(*)(KLEL_VALUE **, void *))(func))
#define KlelCreateIntegerFunc(name, func) \
  KlelCreateValue(KLEL_EXPR_FUNCTION_INTEGER, (const char *)name, (KLEL_VALUE *(*)(KLEL_VALUE **, void *))(func))
#define KlelCreateStringFunc(name, func) \
  KlelCreateValue(KLEL_EXPR_FUNCTION_STRING, (const char *)name, (KLEL_VALUE *(*)(KLEL_VALUE **, void *))(func))

/*-
 ***********************************************************************
 *
 * Default error-handling callback.
 *
 ***********************************************************************
 */
void KlelDefaultReportError(KLEL_CONTEXT *psContext, const char *pcFormat, ...);

/*-
 ***********************************************************************
 *
 * Execute an expression.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelExecute(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Free a command structure.
 *
 ***********************************************************************
 */
void KlelFreeCommand(KLEL_COMMAND *psCommand);

/*-
 ***********************************************************************
 *
 * Free a context.
 *
 ***********************************************************************
 */
void KlelFreeContext(KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Free an expression.
 *
 ***********************************************************************
 */
void KlelFreeNode(KLEL_NODE **psExpression);

/*-
 ***********************************************************************
 *
 * Free a result structure.
 *
 ***********************************************************************
 */
void KlelFreeResult(KLEL_VALUE *psResult);

/*-
 ***********************************************************************
 *
 * Get the checksum of an expression.
 *
 ***********************************************************************
 */
uint32_t KlelGetChecksum(KLEL_NODE *psExpression, unsigned long ulFlags);

/*-
 ***********************************************************************
 *
 * Get the value of a guarded command.
 *
 ***********************************************************************
 */
KLEL_COMMAND *KlelGetCommand(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the interpreter used by a command.
 *
 ***********************************************************************
 */
char *KlelGetCommandInterpreter(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the script or program passed to the interpreter.
 *
 ***********************************************************************
 */
char *KlelGetCommandProgram(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the first error.
 *
 ***********************************************************************
 */
char *KlelGetFirstError(KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the name of the expression.
 *
 ***********************************************************************
 */
char *KlelGetName(KLEL_NODE *psExpression);

/*-
 ***********************************************************************
 *
 * Get the next error.
 *
 ***********************************************************************
 */
char *KlelGetNextError(KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get a release string for the package.
 *
 ***********************************************************************
 */
char *KlelGetRelease(void);

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
 * Get the description of a standard library function.
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE *KlelGetStdLibFuncDesc(const char *pcName, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the type of expression.
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE KlelGetTypeOfExpression(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the type of a standard variable.
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE KlelGetTypeOfStdVar(const char *pcName, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Get the value of a standard variable.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelGetValueOfStdVar(const char *pcName, KLEL_CONTEXT *psContext);

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
 * Determine if an expression is a guarded command.
 *
 ***********************************************************************
 */
int KlelIsGuardedCommand(KLEL_NODE *psExpression);

/*-
 ***********************************************************************
 *
 * Determine the disposition of a return code.
 *
 ***********************************************************************
 */
int KlelIsSuccessReturnCode(KLEL_NODE *psExpression, int code);

/*-
 ***********************************************************************
 *
 * Convert a value into a quoted string.
 *
 ***********************************************************************
 */
char *KlelValueToQuotedString(KLEL_VALUE *psValue, size_t *ulLength);

/*-
 ***********************************************************************
 *
 * Set the quote character.
 *
 ***********************************************************************
 */
void KlelSetQuotedChars(char *c);

/*-
 ***********************************************************************
 *
 * Convert a value into a string.
 *
 ***********************************************************************
 */
char *KlelValueToString(KLEL_VALUE *psValue, size_t *ulLength);

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
char *KlelExpressionToString(KLEL_NODE *psRoot, unsigned long ulFlags);

#endif /* !_KLEL_H_INCLUDED */
