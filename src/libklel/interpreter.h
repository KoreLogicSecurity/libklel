/*-
 ***********************************************************************
 *
 * $Id: interpreter.h,v 1.9 2012/04/26 00:53:31 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _INTERPRETER_H_INCLUDED
#define _INTERPRETER_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Function Dispatch Table Entry Structure
 *
 ***********************************************************************
 */
typedef KLEL_VALUE *(*fInstruction)(KLEL_NODE *, KLEL_CONTEXT *);
typedef struct _KLEL_DISPATCH
{
  const char   *pcName;
  fInstruction  pfInstruction;
} KLEL_DISPATCH;

/*-
 ***********************************************************************
 *
 * Instruction function prototypes.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelDoAdd(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoAnd(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseAnd(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseNot(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseOr(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseXor(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoCall(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoConcat(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoConditional(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoDivide(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoEqual(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoFloat(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoGreaterThan(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoGreaterThanOrEqualTo(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoGuard(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoInteger(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoLessThan(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoLessThanOrEqualTo(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoModulo(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoMultiply(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoNegate(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoNot(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoNotEqual(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoNotRegex(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoOr(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoRegex(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoRollLeft(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoRollRight(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoRoot(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoString(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoSubtract(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoVariable(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);

#endif /* !_INTERPRETER_H_INCLUDED */
