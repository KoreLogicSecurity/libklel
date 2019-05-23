/*-
 ***********************************************************************
 *
 * $Id: interpreter.h,v 1.14 2012/11/14 01:43:25 klm Exp $
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
 * Instruction Functions
 *
 ***********************************************************************
 */
typedef KLEL_VALUE *(*KLEL_INSTRUCTION)(KLEL_NODE *, KLEL_CONTEXT *);

/*-
 ***********************************************************************
 *
 * Instruction function prototypes.
 *
 ***********************************************************************
 */
KLEL_VALUE *KlelDoAdd(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseAnd(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseNot(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseOr(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBitwiseXor(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBooleanAnd(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBooleanNot(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoBooleanOr(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoCall(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoConcat(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoConditional(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoDesignator(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoDivide(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoEqual(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoExpression(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoFragment(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoGreaterThan(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoGreaterThanOrEqual(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoGuardedCommand(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoInteger(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoInterp(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoLessThan(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoLessThanOrEqual(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoLet(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoLike(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoModulus(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoMultiply(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoNegate(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoNotEqual(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoReal(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoRollLeft(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoRollRight(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);
KLEL_VALUE *KlelDoSubtract(KLEL_NODE *psNode, KLEL_CONTEXT *psContext);

/*-
 ***********************************************************************
 *
 * Simmple Instruction Function Construction Macros
 * These macros create instruction functions for "simple" instructions
 * that differ only slightly from each other.
 *
 ***********************************************************************
 */
#define KLEL_SIMPLE_UNARY_OPERATION(name, operation)                                                                              \
  KLEL_VALUE *                                                                                                                    \
  name (KLEL_NODE *psNode, KLEL_CONTEXT *psContext)                                                                               \
  {                                                                                                                               \
    KLEL_VALUE *psLeft   = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext);                                 \
    KLEL_VALUE *psResult = NULL;                                                                                                  \
    KLEL_ASSERT(psNode    != NULL);                                                                                               \
    KLEL_ASSERT(psContext != NULL);                                                                                               \
    if (psLeft == NULL)                                                                                                           \
    {                                                                                                                             \
      return NULL;                                                                                                                \
    }                                                                                                                             \
    psResult = (operation);                                                                                                       \
    KlelFreeResult(psLeft);                                                                                                       \
    return psResult;                                                                                                              \
  }

#define KLEL_SIMPLE_VALUE_OPERATION(name, operation)                                                                              \
  KLEL_VALUE *                                                                                                                    \
  name (KLEL_NODE *psLeft, KLEL_CONTEXT *psContext)                                                                               \
  {                                                                                                                               \
    KLEL_VALUE *psResult = NULL;                                                                                                  \
    KLEL_ASSERT(psLeft    != NULL);                                                                                               \
    KLEL_ASSERT(psContext != NULL);                                                                                               \
    if (psLeft == NULL)                                                                                                           \
    {                                                                                                                             \
      return NULL;                                                                                                                \
    }                                                                                                                             \
    psResult = (operation);                                                                                                       \
    return psResult;                                                                                                              \
  }

#define KLEL_SIMPLE_COMPARISON_OPERATION(name, op, eq)                                                                            \
  KLEL_VALUE *                                                                                                                    \
  name (KLEL_NODE *psNode, KLEL_CONTEXT *psContext)                                                                               \
  {                                                                                                                               \
    KLEL_VALUE *psLeft   = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext);                                 \
    KLEL_VALUE *psRight  = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext);                                 \
    int        bEqual    = 0;                                                                                                     \
    if (psLeft == NULL || psRight == NULL)                                                                                        \
    {                                                                                                                             \
      KlelFreeResult(psLeft);                                                                                                     \
      KlelFreeResult(psRight);                                                                                                    \
      return NULL;                                                                                                                \
    }                                                                                                                             \
    if (psLeft->iType == psRight->iType)                                                                                          \
    {                                                                                                                             \
      switch (psLeft->iType)                                                                                                      \
      {                                                                                                                           \
        case KLEL_TYPE_BOOLEAN:                                                                                                   \
          bEqual = (psLeft->bBoolean op psRight->bBoolean);                                                                       \
          break;                                                                                                                  \
                                                                                                                                  \
        case KLEL_TYPE_INT64:                                                                                                   \
          bEqual = (psLeft->llInteger op psRight->llInteger);                                                                     \
          break;                                                                                                                  \
                                                                                                                                  \
        case KLEL_TYPE_REAL:                                                                                                      \
          bEqual = (psLeft->dReal op psRight->dReal);                                                                             \
          break;                                                                                                                  \
                                                                                                                                  \
        case KLEL_TYPE_STRING:                                                                                                    \
          bEqual = (psLeft->szLength op psRight->szLength eq memcmp(psLeft->acString, psRight->acString, psLeft->szLength) op 0); \
          break;                                                                                                                  \
      }                                                                                                                           \
    }                                                                                                                             \
    KlelFreeResult(psLeft);                                                                                                       \
    KlelFreeResult(psRight);                                                                                                      \
    return KlelCreateBoolean(bEqual);                                                                                             \
  }

#define KLEL_SIMPLE_BINARY_OPERATION(name, expr)                                                                                  \
  KLEL_VALUE *                                                                                                                    \
  name (KLEL_NODE *psNode, KLEL_CONTEXT *psContext)                                                                               \
  {                                                                                                                               \
    KLEL_VALUE *psLeft   = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext);                                 \
    KLEL_VALUE *psRight  = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext);                                 \
    KLEL_VALUE *psResult = NULL;                                                                                                  \
    if (psLeft == NULL || psRight == NULL)                                                                                        \
    {                                                                                                                             \
      KlelFreeResult(psLeft);                                                                                                     \
      KlelFreeResult(psRight);                                                                                                    \
      return NULL;                                                                                                                \
    }                                                                                                                             \
    psResult = (expr);                                                                                                            \
    KlelFreeResult(psLeft);                                                                                                       \
    KlelFreeResult(psRight);                                                                                                      \
    return psResult;                                                                                                              \
  }

#endif /* ! _INTERPRETER_H_INCLUDED */
