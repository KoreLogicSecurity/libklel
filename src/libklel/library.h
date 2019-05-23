/*-
 ***********************************************************************
 *
 * $Id: library.h,v 1.17 2012/11/28 21:52:12 rking Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _LIBRARY_H_INCLUDED
#define _LIBRARY_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Standard library entry structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_STANDARD_LIBRARY_ENTRY
{
  const char     *pcName;
  KLEL_EXPR_TYPE  iType;
  int             bBoolean;
  double          dReal;
  KLEL_VALUE     *(*fFunction)(KLEL_VALUE **, void *);
  int64_t         llInteger;
  const char     *pcString;
  KLEL_EXPR_TYPE  aiArguments[KLEL_MAX_FUNC_ARGS];
} KLEL_STANDARD_LIBRARY_ENTRY;

/*-
 ***********************************************************************
 *
 * Standard library function prototypes.
 *
 ***********************************************************************
 */
KLEL_VALUE     *KlelStdLibAbort(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibAbs(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibChr(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibCksum(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibCtime(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibEntier(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibReal(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibHexOfInt(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibMax(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibMin(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibNow(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibOctOfInt(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibOdd(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibOrd(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibStringOfBool(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibStringOfReal(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibStringOfInt(KLEL_VALUE **asArgs, void *pvContext);
KLEL_VALUE     *KlelStdLibStrlen(KLEL_VALUE **asArgs, void *pvContext);
KLEL_EXPR_TYPE  KlelGetTypeOfStdVar(const char *pcName, void *pvContext);
KLEL_VALUE     *KlelGetValueOfStdVar(const char *pcName, void *pvContext);

#endif /* !_LIBRARY_H_INCLUDED */
