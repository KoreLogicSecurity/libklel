/*-
 ***********************************************************************
 *
 * $Id: typecheck.h,v 1.14 2012/11/14 01:43:25 klm Exp $
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
KLEL_EXPR_TYPE KlelTypeCheck(KLEL_NODE *psNode,               KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckCall(KLEL_NODE *psNode,           KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckConditional(KLEL_NODE *psNode,    KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckDesignator(KLEL_NODE *psNode,     KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckExpression(KLEL_NODE *psNode,     KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckGuardedCommand(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckInterp(KLEL_NODE *psNode,         KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckLet(KLEL_NODE *psNode,            KLEL_CONTEXT *psContext);
KLEL_EXPR_TYPE KlelTypeCheckLike(KLEL_NODE *psNode,           KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Type checking rule entry structure.
 *
 ***********************************************************************
 */
typedef struct _KLEL_TYPE_DESC
{
  KLEL_NODE_TYPE iType;
  KLEL_EXPR_TYPE (*pfFunction)(KLEL_NODE *, KLEL_CONTEXT *);
  KLEL_EXPR_TYPE iOperand1;
  KLEL_EXPR_TYPE iOperand2;
  KLEL_EXPR_TYPE iResult;
  const char     *pcError;
} KLEL_TYPE_DESC;

#endif /* ! _TYPECHECK_H_INCLUDED */
