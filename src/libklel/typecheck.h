/*-
 ***********************************************************************
 *
 * $Id: typecheck.h,v 1.16 2019/07/31 15:59:27 klm Exp $
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
