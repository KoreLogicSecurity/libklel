/*-
 ***********************************************************************
 *
 * $Id: typecheck.h,v 1.10 2012/04/26 00:53:31 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _TYPECHECK_H_INCLUDED
#define _TYPECHECK_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Specialized type checking function prototypes.
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE KlelTypeCheckCall(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckGuard(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckString(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckVar(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Type checking rule entry structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_TYPE_DESC
{
  const char     *pcName;
  KLEL_EXPR_TYPE (*fFunc)(KLEL_NODE *, KLEL_CONTEXT *);
  KLEL_EXPR_TYPE  aiChildren[KLEL_MAX_CHILDREN];
  KLEL_EXPR_TYPE  iType;
  const char     *pcError;
} KLEL_TYPE_DESC;

#endif /* !_TYPECHECK_H_INCLUDED */
