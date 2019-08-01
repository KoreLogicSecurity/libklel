/*-
 ***********************************************************************
 *
 * $Id: api.h,v 1.14 2019/07/31 15:59:27 klm Exp $
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
#ifndef _API_H_INCLUDED
#define _API_H_INCLUDED

#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * Internal function prototypes.
 *
 ***********************************************************************
 */
KLEL_VALUE     *KlelInnerExecute(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);
KLEL_NODE      *KlelParse(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheck(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);
void           KlelFreeNode(KLEL_NODE *psNode);
KLEL_EXPR_TYPE KlelInnerGetTypeOfVar(KLEL_CONTEXT *psContext, const char *pcName, void *pvData);
KLEL_VALUE     *KlelInnerGetValueOfVar(KLEL_CONTEXT *psContext, const char *pcName, void *pvData);
int            KlelIsConstantString(KLEL_NODE *psNode);
size_t         KlelConstantStringLength(KLEL_NODE *psNode);

#endif /* !_API_H_INCLUDED */
