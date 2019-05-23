/*-
 ***********************************************************************
 *
 * $Id: library.h,v 1.12 2012/04/26 00:53:31 klm Exp $
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
  double          dFloat;
  KLEL_VALUE     *(*fFunction)(KLEL_VALUE **, KLEL_CONTEXT *);
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
KLEL_VALUE *KlelStdLibAbs(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibAbs(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibChr(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibCksum(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibEntier(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibFloat(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibHexOfInt(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibMax(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibMin(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibNow(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibOctOfInt(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibOdd(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibOrd(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibStringOfBool(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibStringOfFloat(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibStringOfInt(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelStdLibStrlen(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);

#endif /* !_LIBRARY_H_INCLUDED */
