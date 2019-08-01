/*-
 ***********************************************************************
 *
 * $Id: library.h,v 1.19 2019/07/31 15:59:27 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2019 The KL-EL Project, All Rights Reserved.
 *
 * This software, having been partly or wholly developed and/or
 * sponsored by KoreLogic, Inc., is hereby released under the terms
 * and conditions set forth in the project's "README.LICENSE" file.
 * For a list of all contributors and sponsors, please refer to the
 * project's "README.CREDITS" file.
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
