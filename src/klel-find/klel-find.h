/*-
 ***********************************************************************
 *
 * $Id: klel-find.h,v 1.11 2019/07/31 15:59:27 klm Exp $
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
