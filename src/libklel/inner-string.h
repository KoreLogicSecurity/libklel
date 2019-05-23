/*-
 ***********************************************************************
 *
 * $Id: inner-string.h,v 1.12 2012/11/14 01:43:25 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _INNER_STRING_H_INCLUDED
#define _INNER_STRING_H_INCLUDED

#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * The string structure.
 *
 ***********************************************************************
 */
#define KLEL_INITIAL_STRING_SIZE 1024

typedef struct _KLEL_STRING
{
  char   *pcString;
  size_t szAlloc;
  size_t szLength;
} KLEL_STRING;

/*-
 ***********************************************************************
 *
 * Allocate and free strings.
 *
 ***********************************************************************
 */
KLEL_STRING *KlelStringNew(void);
void KlelStringFree(KLEL_STRING *psString, int bFreeString);

/*-
 ***********************************************************************
 *
 * Grow strings.
 *
 ***********************************************************************
 */
int KlelStringReserve(KLEL_STRING *psDest, size_t szLength);

/*-
 ***********************************************************************
 *
 * Modify strings.
 *
 ***********************************************************************
 */
KLEL_STRING *KlelStringConcat(KLEL_STRING *psDest, KLEL_STRING *psSource);
KLEL_STRING *KlelStringConcatCString(KLEL_STRING *psDest, const char *pcSource);
int KlelStringPrintf(KLEL_STRING *psString, const char *pcFormat, ...);

/*-
 ***********************************************************************
 *
 * String conversion function table structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_NODE_STRING_MAP
{
  KLEL_STRING *(*pfInstruction)(KLEL_NODE *, const char *, unsigned long);
  const char  *pcString;
} KLEL_NODE_STRING_MAP;

/*-
 ***********************************************************************
 *
 * Node to string conversion functions.
 *
 ***********************************************************************
 */
KLEL_STRING *KlelInnerStringOfExpression(KLEL_NODE *psNode, unsigned long ulFlags);
KLEL_STRING *KlelStringOfBinaryOp(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfCall(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfConditional(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfDesignator(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfExpression(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfFragment(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfGuardedCommand(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfInteger(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfInterpolation(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfLet(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfReal(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringOfUnaryOp(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);

#endif /* !_INNER_STRING_H_INCLUDED */
