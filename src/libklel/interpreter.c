/*-
 ***********************************************************************
 *
 * $Id: interpreter.c,v 1.55 2012/11/15 23:47:23 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * Function Dispatch Table
 *
 ***********************************************************************
 */
static KLEL_INSTRUCTION gpfKlelDispatch[] =
{
  [KLEL_NODE_AND]             = KlelDoBitwiseAnd,
  [KLEL_NODE_AND_AND]         = KlelDoBooleanAnd,
  [KLEL_NODE_BANG]            = KlelDoBooleanNot,
  [KLEL_NODE_CALL]            = KlelDoCall,
  [KLEL_NODE_CARET]           = KlelDoBitwiseXor,
  [KLEL_NODE_CONDITIONAL]     = KlelDoConditional,
  [KLEL_NODE_DESIGNATOR]      = KlelDoDesignator,
  [KLEL_NODE_DOT]             = KlelDoConcat,
  [KLEL_NODE_EQ_EQ]           = KlelDoEqual,
  [KLEL_NODE_EXPRESSION]      = KlelDoExpression,
  [KLEL_NODE_FRAGMENT]        = KlelDoFragment,
  [KLEL_NODE_GT]              = KlelDoGreaterThan,
  [KLEL_NODE_GTE]             = KlelDoGreaterThanOrEqual,
  [KLEL_NODE_GT_GT]           = KlelDoRollRight,
  [KLEL_NODE_GUARDED_COMMAND] = KlelDoGuardedCommand,
  [KLEL_NODE_INTEGER]         = KlelDoInteger,
  [KLEL_NODE_INTERP]          = KlelDoInterp,
  [KLEL_NODE_LET]             = KlelDoLet,
  [KLEL_NODE_LIKE]            = KlelDoLike,
  [KLEL_NODE_LT]              = KlelDoLessThan,
  [KLEL_NODE_LTE]             = KlelDoLessThanOrEqual,
  [KLEL_NODE_LT_LT]           = KlelDoRollLeft,
  [KLEL_NODE_MINUS]           = KlelDoSubtract,
  [KLEL_NODE_NE]              = KlelDoNotEqual,
  [KLEL_NODE_NEGATE]          = KlelDoNegate,
  [KLEL_NODE_PERCENT]         = KlelDoModulus,
  [KLEL_NODE_PIPE]            = KlelDoBitwiseOr,
  [KLEL_NODE_PIPE_PIPE]       = KlelDoBooleanOr,
  [KLEL_NODE_PLUS]            = KlelDoAdd,
  [KLEL_NODE_QUOTED_INTERP]   = KlelDoInterp,
  [KLEL_NODE_REAL]            = KlelDoReal,
  [KLEL_NODE_SLASH]           = KlelDoDivide,
  [KLEL_NODE_STAR]            = KlelDoMultiply,
  [KLEL_NODE_TILDE]           = KlelDoBitwiseNot,
  [KLEL_NODE_UNLIKE]          = KlelDoLike
};


/*-
 ***********************************************************************
 *
 * KlelInnerExecute
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelInnerExecute(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot    != NULL);
  KLEL_ASSERT(psContext != NULL);

  if (psRoot == NULL)
  {
    KlelReportError(psContext, "%s", "node is null", NULL);
    return NULL;
  }

  if (psRoot->iType > KLEL_NODE_UNLIKE || gpfKlelDispatch[psRoot->iType] == NULL)
  {
    KlelReportError(psContext, "%s", "invalid instruction", NULL);
    return NULL;
  }

  return gpfKlelDispatch[psRoot->iType](psRoot, psContext);
}


/*-
 ***********************************************************************
 *
 * The Simple Unary and Value Operations
 *
 ***********************************************************************
 */
KLEL_SIMPLE_UNARY_OPERATION(KlelDoBitwiseNot, KlelCreateInteger(~(psLeft->bBoolean)));
KLEL_SIMPLE_UNARY_OPERATION(KlelDoBooleanNot, KlelCreateBoolean(!(psLeft->bBoolean)));
KLEL_SIMPLE_UNARY_OPERATION(KlelDoNegate,     psLeft->iType == KLEL_TYPE_INT64 ? KlelCreateInteger(-(psLeft->llInteger)) : KlelCreateReal(-(psLeft->dReal)));
KLEL_SIMPLE_VALUE_OPERATION(KlelDoFragment,   KlelCreateString(psLeft->szLength, psLeft->acFragment));
KLEL_SIMPLE_VALUE_OPERATION(KlelDoInteger,    KlelCreateInteger(psLeft->llInteger));
KLEL_SIMPLE_VALUE_OPERATION(KlelDoReal,       KlelCreateReal(psLeft->dReal));


/*-
 ***********************************************************************
 *
 * The Simple Comparison Operations
 *
 ***********************************************************************
 */
KLEL_SIMPLE_COMPARISON_OPERATION(KlelDoEqual,              ==, &&);
KLEL_SIMPLE_COMPARISON_OPERATION(KlelDoGreaterThan,         >, ||);
KLEL_SIMPLE_COMPARISON_OPERATION(KlelDoGreaterThanOrEqual, >=, ||);
KLEL_SIMPLE_COMPARISON_OPERATION(KlelDoLessThan,            <, ||);
KLEL_SIMPLE_COMPARISON_OPERATION(KlelDoLessThanOrEqual,    <=, ||);
KLEL_SIMPLE_COMPARISON_OPERATION(KlelDoNotEqual,           !=, ||);


/*-
 ***********************************************************************
 *
 * The Simple Binary Operations
 *
 ***********************************************************************
 */
KLEL_SIMPLE_BINARY_OPERATION(KlelDoBitwiseAnd, KlelCreateInteger(psLeft->llInteger &  psRight->llInteger));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoBitwiseOr,  KlelCreateInteger(psLeft->llInteger |  psRight->llInteger));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoBitwiseXor, KlelCreateInteger(psLeft->llInteger ^  psRight->llInteger));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoBooleanAnd, KlelCreateBoolean(psLeft->bBoolean && psRight->bBoolean));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoBooleanOr,  KlelCreateBoolean(psLeft->bBoolean || psRight->bBoolean));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoModulus,    (psRight->llInteger == 0 ? KlelReportError(psContext, "divide by zero", NULL), NULL
                                                                        : KlelCreateInteger(psLeft->llInteger %  psRight->llInteger)));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoRollLeft,   KlelCreateInteger(psLeft->llInteger << psRight->llInteger));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoRollRight,  KlelCreateInteger(psLeft->llInteger >> psRight->llInteger));
KLEL_SIMPLE_BINARY_OPERATION(KlelDoAdd,        (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_INT64)   ?
                                                   KlelCreateInteger(psLeft->llInteger + psRight->llInteger)
                                               : (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   KlelCreateReal(psLeft->llInteger + psRight->dReal)
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_INT64) ?
                                                   KlelCreateReal(psLeft->dReal + psRight->llInteger)
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   KlelCreateReal(psLeft->dReal + psRight->dReal)
                                               : NULL);
KLEL_SIMPLE_BINARY_OPERATION(KlelDoDivide,     (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_INT64)   ?
                                                   (psRight->llInteger == 0 ? KlelReportError(psContext, "divide by zero", NULL), NULL
                                                                            : KlelCreateInteger(psLeft->llInteger / psRight->llInteger))
                                               : (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   (psRight->dReal == 0.0 ? KlelReportError(psContext, "divide by zero", NULL), NULL
                                                                           : KlelCreateReal(psLeft->llInteger / psRight->dReal))
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_INT64) ?
                                                   (psRight->llInteger == 0 ? KlelReportError(psContext, "divide by zero", NULL), NULL
                                                                            : KlelCreateReal(psLeft->dReal / psRight->llInteger))
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   (psRight->dReal == 0.0 ? KlelReportError(psContext, "divide by zero", NULL), NULL
                                                                           : KlelCreateReal(psLeft->dReal / psRight->dReal))
                                               : NULL);
KLEL_SIMPLE_BINARY_OPERATION(KlelDoMultiply,   (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_INT64)   ?
                                                   KlelCreateInteger(psLeft->llInteger * psRight->llInteger)
                                               : (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   KlelCreateReal(psLeft->llInteger * psRight->dReal)
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_INT64) ?
                                                   KlelCreateReal(psLeft->dReal * psRight->llInteger)
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   KlelCreateReal(psLeft->dReal * psRight->dReal)
                                               : NULL);
KLEL_SIMPLE_BINARY_OPERATION(KlelDoSubtract,   (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_INT64)   ?
                                                   KlelCreateInteger(psLeft->llInteger - psRight->llInteger)
                                               : (psLeft->iType == KLEL_TYPE_INT64 && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   KlelCreateReal(psLeft->llInteger - psRight->dReal)
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_INT64) ?
                                                   KlelCreateReal(psLeft->dReal - psRight->llInteger)
                                               : (psLeft->iType == KLEL_TYPE_REAL    && psRight->iType == KLEL_TYPE_REAL)    ?
                                                   KlelCreateReal(psLeft->dReal - psRight->dReal)
                                               : NULL);


/*-
 ***********************************************************************
 *
 * KlelDoCall
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoCall(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psFunction                       = KlelInnerGetValueOfVar(psContext, psNode->acFragment, psContext->pvData);
  KLEL_VALUE *apsArguments[KLEL_MAX_FUNC_ARGS] = {0};
  KLEL_VALUE *psResult                         = NULL;

  apsArguments[0]  = (psNode->apsChildren[KLEL_ARGUMENT1_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT1_INDEX],  psContext) : NULL;
  apsArguments[1]  = (psNode->apsChildren[KLEL_ARGUMENT2_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT2_INDEX],  psContext) : NULL;
  apsArguments[2]  = (psNode->apsChildren[KLEL_ARGUMENT3_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT3_INDEX],  psContext) : NULL;
  apsArguments[3]  = (psNode->apsChildren[KLEL_ARGUMENT4_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT4_INDEX],  psContext) : NULL;
  apsArguments[4]  = (psNode->apsChildren[KLEL_ARGUMENT5_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT5_INDEX],  psContext) : NULL;
  apsArguments[5]  = (psNode->apsChildren[KLEL_ARGUMENT6_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT6_INDEX],  psContext) : NULL;
  apsArguments[6]  = (psNode->apsChildren[KLEL_ARGUMENT7_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT7_INDEX],  psContext) : NULL;
  apsArguments[7]  = (psNode->apsChildren[KLEL_ARGUMENT8_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT8_INDEX],  psContext) : NULL;
  apsArguments[8]  = (psNode->apsChildren[KLEL_ARGUMENT9_INDEX]  != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT9_INDEX],  psContext) : NULL;
  apsArguments[9]  = (psNode->apsChildren[KLEL_ARGUMENT10_INDEX] != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT10_INDEX], psContext) : NULL;
  apsArguments[10] = (psNode->apsChildren[KLEL_ARGUMENT11_INDEX] != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT11_INDEX], psContext) : NULL;
  apsArguments[11] = (psNode->apsChildren[KLEL_ARGUMENT12_INDEX] != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT12_INDEX], psContext) : NULL;
  apsArguments[12] = (psNode->apsChildren[KLEL_ARGUMENT13_INDEX] != NULL) ? KlelInnerExecute(psNode->apsChildren[KLEL_ARGUMENT13_INDEX], psContext) : NULL;

  if (psFunction != NULL)
  {
    psResult = psFunction->fFunction(apsArguments, (void *)psContext);
    KlelFreeResult(psFunction);
  }

  apsArguments[0]  != NULL ? KlelFreeResult(apsArguments[0])  : NULL;
  apsArguments[1]  != NULL ? KlelFreeResult(apsArguments[1])  : NULL;
  apsArguments[2]  != NULL ? KlelFreeResult(apsArguments[2])  : NULL;
  apsArguments[3]  != NULL ? KlelFreeResult(apsArguments[3])  : NULL;
  apsArguments[4]  != NULL ? KlelFreeResult(apsArguments[4])  : NULL;
  apsArguments[5]  != NULL ? KlelFreeResult(apsArguments[5])  : NULL;
  apsArguments[6]  != NULL ? KlelFreeResult(apsArguments[6])  : NULL;
  apsArguments[7]  != NULL ? KlelFreeResult(apsArguments[7])  : NULL;
  apsArguments[8]  != NULL ? KlelFreeResult(apsArguments[8])  : NULL;
  apsArguments[9]  != NULL ? KlelFreeResult(apsArguments[9])  : NULL;
  apsArguments[10] != NULL ? KlelFreeResult(apsArguments[10]) : NULL;
  apsArguments[11] != NULL ? KlelFreeResult(apsArguments[11]) : NULL;
  apsArguments[12] != NULL ? KlelFreeResult(apsArguments[12]) : NULL;

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoConcat
 * FIXME - This is the most inefficient operation.
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoConcat(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext);
  KLEL_VALUE *psRight  = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext);
  char       *pcString = NULL;
  KLEL_VALUE *psResult = NULL;

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);
    return NULL;
  }

  pcString = calloc(1, sizeof(KLEL_VALUE) + psLeft->szLength + psRight->szLength + 1);
  if (pcString == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);
    return NULL;
  }

  memcpy(pcString, psLeft->acString, psLeft->szLength);
  memcpy(pcString + psLeft->szLength, psRight->acString, psRight->szLength);

  psResult = KlelCreateString(psLeft->szLength + psRight->szLength, pcString);
  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  free(pcString);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoConditional
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoConditional(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psPredicate = NULL;
  KLEL_VALUE *psResult    = NULL;

  psPredicate = KlelInnerExecute(psNode->apsChildren[KLEL_PREDICATE_INDEX], psContext);
  if (psPredicate == NULL)
  {
    KlelFreeResult(psPredicate);
    return NULL;
  }

  psResult = KlelInnerExecute(psNode->apsChildren[psPredicate->bBoolean ? KLEL_IFTRUE_INDEX : KLEL_IFFALSE_INDEX], psContext);

  KlelFreeResult(psPredicate);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoDesignator
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoDesignator(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  if (psNode->iClosure < 0)
  {
    return KlelInnerGetValueOfVar(psContext, psNode->acFragment, psContext->pvData);
  }

  switch (psContext->psClosures[psNode->iClosure].iType)
  {
    case KLEL_TYPE_BOOLEAN:
      return KlelCreateBoolean(psContext->psClosures[psNode->iClosure].psValue->bBoolean);

    case KLEL_TYPE_INT64:
      return KlelCreateInteger(psContext->psClosures[psNode->iClosure].psValue->llInteger);

    case KLEL_TYPE_REAL:
      return KlelCreateReal(psContext->psClosures[psNode->iClosure].psValue->dReal);

    case KLEL_TYPE_STRING:
      return KlelCreateString(psContext->psClosures[psNode->iClosure].psValue->szLength, psContext->psClosures[psNode->iClosure].psValue->acString);

    default:
      KLEL_ASSERT(0);
      return NULL;
  }

  KLEL_ASSERT(0);
  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelDoExpression
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoExpression(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  return KlelInnerExecute(psNode->apsChildren[KLEL_EXPRESSION_INDEX], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelDoGuardedCommand
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoGuardedCommand(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  return KlelInnerExecute(psNode->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_PREDICATE_INDEX], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelDoInterp
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoInterp(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psValue  = KlelDoDesignator(psNode, psContext);
  KLEL_VALUE *psResult = NULL;
  size_t     szLength  = 0;
  char       *pcString = NULL;

  if (psValue != NULL)
  {
    pcString = psNode->iType == KLEL_NODE_INTERP ? KlelValueToString(psValue, &szLength) : KlelValueToQuotedString(psValue, &szLength);
    if (pcString != NULL)
    {
      psResult = KlelCreateString(szLength, pcString);
      free(pcString);
    }
  }

  KlelFreeResult(psValue);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoLet
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoLet(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  if (psContext->psClosures[psNode->iClosure].psValue != NULL)
  {
    KlelFreeResult(psContext->psClosures[psNode->iClosure].psValue);
  }

  psContext->psClosures[psNode->iClosure].psValue = KlelInnerExecute(psNode->apsChildren[KLEL_DEFINITION_INDEX], psContext);
  return KlelInnerExecute(psNode->apsChildren[KLEL_EXPRESSION_INDEX], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelDoLike
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoLike(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psString     = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext);
  KLEL_VALUE *psRegex      = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext);
  pcre       *psExpression = NULL;
  const char *pcError      = NULL;
  int        iOffset       = 0;
  int        iMatches[2]   = {0};
  int        iCount        = 0;

  if (psString == NULL || psRegex == NULL)
  {
    KlelFreeResult(psString);
    KlelFreeResult(psRegex);
    return NULL;
  }

  psExpression = pcre_compile(psRegex->acString, 0, &pcError, &iOffset, NULL); /* FIXME - We should cache this for constant strings. */
  if (psExpression == NULL)
  {
    KlelReportError(psContext, "regular expression failed: %s", pcError, NULL);
    KlelFreeResult(psString);
    KlelFreeResult(psRegex);
    return NULL;
  }

  iCount = pcre_exec(psExpression, NULL, psString->acString, psString->szLength, 0, 0, iMatches, 2);

  pcre_free(psExpression);
  KlelFreeResult(psString);
  KlelFreeResult(psRegex);

  return KlelCreateBoolean(psNode->iType == KLEL_NODE_LIKE ? iCount >= 0 : iCount < 0);
}
