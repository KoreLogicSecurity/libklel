/*-
 ***********************************************************************
 *
 * $Id: klel-find.h,v 1.9 2012/11/14 01:43:25 klm Exp $
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
KLEL_VALUE *KlelFindBasename(KLEL_VALUE **asArgs, void *pvData);
KLEL_EXPR_TYPE *GetFuncDesc(const char *pcName, void *pvData);
KLEL_EXPR_TYPE GetTypeOfVar(const char *pcName, void *pvData);
KLEL_VALUE *GetValueOfVar(const char *pcName, void *pvData);

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
