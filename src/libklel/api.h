/*-
 ***********************************************************************
 *
 * $Id: api.h,v 1.8 2012/04/26 00:53:31 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
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
KLEL_STRING *KlelInnerExpressionToString(KLEL_NODE *psNode, unsigned long ulFlags);
KLEL_VALUE *KlelInnerExecute(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelParse(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheck(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);

#endif /* !_API_H_INCLUDED */
