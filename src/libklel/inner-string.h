/*-
 ***********************************************************************
 *
 * $Id: inner-string.h,v 1.9 2012/04/26 00:53:31 klm Exp $
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
  const char  *pcName;
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
KLEL_STRING *KlelArgsToString(KLEL_NODE *psNode,        const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelCallToString(KLEL_NODE *psNode,        const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelConditionalToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelFloatToString(KLEL_NODE *psNode,       const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelGuardToString(KLEL_NODE *psNode,       const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelIntegerToString(KLEL_NODE *psNode,     const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelOpToString(KLEL_NODE *psNode,          const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelRootToString(KLEL_NODE *psNode,        const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelStringToString(KLEL_NODE *psNode,      const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelUnopToString(KLEL_NODE *psNode,        const char *pcString, unsigned long ulFlags);
KLEL_STRING *KlelVarToString(KLEL_NODE *psNode,         const char *pcString, unsigned long ulFlags);

#endif /* !_INNER_STRING_H_INCLUDED */
