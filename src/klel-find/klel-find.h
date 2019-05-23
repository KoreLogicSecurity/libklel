/*-
 ***********************************************************************
 *
 * $Id: klel-find.h,v 1.7 2012/04/26 00:53:31 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _KLEL_FIND_H_INCLUDED
#define _KLEL_FIND_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Defines
 *
 ***********************************************************************
 */
#define PROGRAM_NAME "klel-find"

/*-
 ***********************************************************************
 *
 * Internal function prototypes.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelFindBasename(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE *GetFuncDesc(const char *pcName, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE GetTypeOfVar(const char *pcName, KLEL_CONTEXT *psContext);
KLEL_VALUE *GetValueOfVar(const char *pcName, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Exported function lookup table entry structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_FUNC_DESC
{
  const char     *pcName;
  KLEL_EXPR_TYPE  aiArguments[KLEL_MAX_FUNC_ARGS];
} KLEL_FUNC_DESC;

/*-
 ***********************************************************************
 *
 * Exported value type descriptor.
 *
 ***********************************************************************
 */
typedef struct _KLEL_TYPE_SPEC
{
  const char     *pcName;
  KLEL_EXPR_TYPE  iType;
} KLEL_TYPE_SPEC;

/*-
 ***********************************************************************
 *
 * Data structure passed to nftw(3).
 *
 ***********************************************************************
 */
typedef struct _KLEL_FIND_ITEM
{
  const char        *pcName;
  const struct stat *psStat;
} KLEL_FIND_ITEM;

#endif /* !_KLEL_FIND_H_INCLUDED */
