/*-
 ***********************************************************************
 *
 * $Id: interpreter.c,v 1.38 2012/04/23 16:40:22 klm Exp $
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
static KLEL_DISPATCH gasKlelDispatch[] =
{
  {"+",         KlelDoAdd},
  {"&&",        KlelDoAnd},
  {"&",         KlelDoBitwiseAnd},
  {"bnot",      KlelDoBitwiseNot},
  {"|",         KlelDoBitwiseOr},
  {"^",         KlelDoBitwiseXor},
  {".",         KlelDoConcat},
  {"cond",      KlelDoConditional},
  {"call",      KlelDoCall},
  {"/",         KlelDoDivide},
  {"==",        KlelDoEqual},
  {"float",     KlelDoFloat},
  {">",         KlelDoGreaterThan},
  {">=",        KlelDoGreaterThanOrEqualTo},
  {"guard",     KlelDoGuard},
  {"int",       KlelDoInteger},
  {"<",         KlelDoLessThan},
  {"<=",        KlelDoLessThanOrEqualTo},
  {"%",         KlelDoModulo},
  {"*",         KlelDoMultiply},
  {"neg",       KlelDoNegate},
  {"not",       KlelDoNot},
  {"!=",        KlelDoNotEqual},
  {"!~",        KlelDoNotRegex},
  {"||",        KlelDoOr},
  {"=~",        KlelDoRegex},
  {"root",      KlelDoRoot},
  {"<<",        KlelDoRollLeft},
  {">>",        KlelDoRollRight},
  {"-",         KlelDoSubtract},
  {"quote",     KlelDoVariable},
  {"string",    KlelDoString},
  {"var",       KlelDoVariable}
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
  size_t szi = 0;

  KLEL_ASSERT(psRoot != NULL);

  if (psRoot == NULL)
  {
    KlelReportError(psContext, "%s", "node is null", NULL);
    return NULL;
  }

  for (szi = 0; szi < sizeof(gasKlelDispatch) / sizeof(KLEL_DISPATCH); szi++)
  {
    if (strcmp(gasKlelDispatch[szi].pcName, psRoot->acName) == 0)
    {
      return gasKlelDispatch[szi].pfInstruction(psRoot, psContext);
    }
  }

  KLEL_ASSERT(0); /* Making it here means an invalid opcode and bug in the compiler. */
  KlelReportError(psContext, "%s", "illegal instruction", NULL);
  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelDoFloat
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoFloat(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot                          != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "float") == 0);

  return KlelCreateFloat(psRoot->dFloat);
}


/*-
 ***********************************************************************
 *
 * KlelDoInteger
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoInteger(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot                        != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "int") == 0);

  return KlelCreateInteger(psRoot->llInteger);
}


/*-
 ***********************************************************************
 *
 * KlelDoConditional
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoConditional(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psPredicate = NULL;
  KLEL_VALUE *psResult    = NULL;

  KLEL_ASSERT(psRoot                         != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]         != NULL); /* The predicate.  */
  KLEL_ASSERT(psRoot->apsChildren[1]         != NULL); /* The true case.  */
  KLEL_ASSERT(psRoot->apsChildren[2]         != NULL); /* The false case. */
  KLEL_ASSERT(strcmp(psRoot->acName, "cond") == 0);

  psPredicate = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  KLEL_ASSERT(psPredicate->iType == KLEL_EXPR_BOOLEAN);

  if (psPredicate == NULL)
  {
    return NULL;
  }

  if (psPredicate->bBoolean)
  {
    psResult = KlelInnerExecute(psRoot->apsChildren[1], psContext);
  }
  else
  {
    psResult = KlelInnerExecute(psRoot->apsChildren[2], psContext);
  }

  KlelFreeResult(psPredicate);
  return psResult;
}

/*-
 ***********************************************************************
 *
 * KlelDoAdd
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoAdd(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "+") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType  == KLEL_EXPR_FLOAT || psLeft->iType  == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psRight->iType == KLEL_EXPR_FLOAT || psRight->iType == KLEL_EXPR_INTEGER);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->dFloat + psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateFloat(psLeft->dFloat + psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->llInteger + psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateInteger(psLeft->llInteger + psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoSubtract
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoSubtract(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "-") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType  == KLEL_EXPR_FLOAT || psLeft->iType  == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psRight->iType == KLEL_EXPR_FLOAT || psRight->iType == KLEL_EXPR_INTEGER);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->dFloat - psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateFloat(psLeft->dFloat - psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->llInteger - psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateInteger(psLeft->llInteger - psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoMultiply
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoMultiply(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "*") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType  == KLEL_EXPR_FLOAT || psLeft->iType  == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psRight->iType == KLEL_EXPR_FLOAT || psRight->iType == KLEL_EXPR_INTEGER);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->dFloat * psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateFloat(psLeft->dFloat * psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->llInteger * psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateInteger(psLeft->llInteger * psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoDivide
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoDivide(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "/") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType  == KLEL_EXPR_FLOAT || psLeft->iType  == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psRight->iType == KLEL_EXPR_FLOAT || psRight->iType == KLEL_EXPR_INTEGER);

  if ((psRight->iType == KLEL_EXPR_FLOAT && psRight->dFloat == 0.0) || (psRight->iType == KLEL_EXPR_INTEGER && psRight->llInteger == 0))
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "divide by zero", psRoot->apsChildren[0]->pcInput, NULL);
    return NULL;
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->dFloat / psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateFloat(psLeft->dFloat / psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(psLeft->llInteger / psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateInteger(psLeft->llInteger / psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoConcat
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoConcat(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;
  char   *pcBuffer     = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, ".") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_STRING && psRight->iType == KLEL_EXPR_STRING);

  pcBuffer = calloc(1, psLeft->szLength + psRight->szLength);
  if (pcBuffer != NULL)
  {
    memcpy(pcBuffer, psLeft->acString, psLeft->szLength);
    memcpy(pcBuffer + psLeft->szLength, psRight->acString, psRight->szLength);
    psResult = KlelCreateString(psLeft->szLength + psRight->szLength, pcBuffer);
    free(pcBuffer);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoModulo
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoModulo(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "%") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER);

  if (psRight->llInteger == 0)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "modulo by zero", psRoot->apsChildren[0]->pcInput, NULL);
    return NULL;
  }

  psResult = KlelCreateInteger(psLeft->llInteger % psRight->llInteger);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoBitwiseAnd
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoBitwiseAnd(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "&") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER);

  psResult = KlelCreateInteger(psLeft->llInteger & psRight->llInteger);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoBitwiseNot
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoBitwiseNot(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                         != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]         != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "bnot") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);

  if (psLeft == NULL)
  {
    KlelFreeResult(psLeft);
    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER);

  psResult = KlelCreateInteger(~(psLeft->llInteger));

  KlelFreeResult(psLeft);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoBitwiseOr
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoBitwiseOr(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "|") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER);

  psResult = KlelCreateInteger(psLeft->llInteger | psRight->llInteger);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoBitwiseXor
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoBitwiseXor(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "^") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER);

  psResult = KlelCreateInteger(psLeft->llInteger ^ psRight->llInteger);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoLessThan
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoLessThan(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "<") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_FLOAT || psLeft->iType == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psLeft->iType == psRight->iType);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat < psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat < psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger < psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger < psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoLessThanOrEqualTo
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoLessThanOrEqualTo(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "<=") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_FLOAT || psLeft->iType == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psLeft->iType == psRight->iType);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat <= psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat <= psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger <= psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger <= psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoGuard
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoGuard(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot                          != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]          != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]          != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "guard") == 0);

  return KlelInnerExecute(psRoot->apsChildren[0], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelDoGreaterThan
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoGreaterThan(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]      != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]      != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, ">") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_FLOAT || psLeft->iType == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psLeft->iType == psRight->iType);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat > psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat > psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger > psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger > psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoGreaterThanOrEqualTo
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoGreaterThanOrEqualTo(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, ">=") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_FLOAT || psLeft->iType == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(psLeft->iType == psRight->iType);

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat >= psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_FLOAT && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->dFloat >= psRight->llInteger);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger >= psRight->dFloat);
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateBoolean(psLeft->llInteger >= psRight->llInteger);
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoEqual
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoEqual(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "==") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == psRight->iType);

  switch (psLeft->iType)
  {
    case KLEL_EXPR_BOOLEAN:
      psResult = KlelCreateBoolean(psLeft->bBoolean == psRight->bBoolean);
      break;

    case KLEL_EXPR_FLOAT:
      psResult = KlelCreateBoolean(psLeft->dFloat == psRight->dFloat);
      break;

    case KLEL_EXPR_INTEGER:
      psResult = KlelCreateBoolean(psLeft->llInteger == psRight->llInteger);
      break;

    case KLEL_EXPR_STRING:
      psResult = KlelCreateBoolean(psLeft->szLength == psRight->szLength && memcmp(psLeft->acString, psRight->acString, psLeft->szLength) == 0);
      break;

    default:
      KLEL_ASSERT(0);
      return NULL;
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoNotEqual
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoNotEqual(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "!=") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == psRight->iType);

  switch (psLeft->iType)
  {
    case KLEL_EXPR_BOOLEAN:
      psResult = KlelCreateBoolean(psLeft->bBoolean != psRight->bBoolean);
      break;

    case KLEL_EXPR_FLOAT:
      psResult = KlelCreateBoolean(psLeft->dFloat != psRight->dFloat);
      break;

    case KLEL_EXPR_INTEGER:
      psResult = KlelCreateBoolean(psLeft->llInteger != psRight->llInteger);
      break;

    case KLEL_EXPR_STRING:
      psResult = KlelCreateBoolean(psLeft->szLength != psRight->szLength || memcmp(psLeft->acString, psRight->acString, psLeft->szLength) != 0);
      break;

    default:
      KLEL_ASSERT(0);
      return NULL;
  }

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoRegex
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoRegex(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  int        iOff       = 0;
  const char *pcErr     = NULL;
  pcre       *psPattern = NULL;
  KLEL_VALUE *psLeft    = NULL;
  int        iMatch     = 0;
  KLEL_VALUE *psRight   = NULL;
  KLEL_VALUE *psResult  = NULL;
  int        aiVec[6]   = {0};

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "=~") == 0 || strcmp(psRoot->acName, "!~") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_STRING && psRight->iType == KLEL_EXPR_STRING);

  psPattern = pcre_compile(psRight->acString, PCRE_DOTALL, &pcErr, &iOff, NULL);
  if (psPattern == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid regular expression", psRight->acString, NULL);
    return NULL;
  }

  iMatch = pcre_exec(psPattern, NULL, psLeft->acString, psLeft->szLength, 0, 0, aiVec, 6);
  psResult = KlelCreateBoolean(iMatch >= 0);

  pcre_free(psPattern);
  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoRollLeft
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoRollLeft(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "<<") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER);

  psResult = KlelCreateInteger(psLeft->llInteger << psRight->llInteger);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}
/*-
 ***********************************************************************
 *
 * KlelDoRoot
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoRoot(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot                                           != NULL);
  KLEL_ASSERT(psRoot->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "root")                   == 0);

  return KlelInnerExecute(psRoot->apsChildren[KLEL_EXPRESSION_CHILD_INDEX], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelDoRollRight
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoRollRight(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, ">>") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_INTEGER && psRight->iType == KLEL_EXPR_INTEGER);

  psResult = KlelCreateInteger(psLeft->llInteger >> psRight->llInteger);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoNotRegex
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoNotRegex(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psResult = KlelDoRegex(psRoot, psContext);

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "!~") == 0);

  if (psResult != NULL)
  {
    psResult->bBoolean = !(psResult->bBoolean);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoAnd
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoAnd(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "&&") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_BOOLEAN && psRight->iType == KLEL_EXPR_BOOLEAN);

  psResult = KlelCreateBoolean(psLeft->bBoolean && psRight->bBoolean);

  KlelFreeResult(psLeft);
  KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoOr
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoOr(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psRight  = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]       != NULL);
  KLEL_ASSERT(psRoot->apsChildren[1]       != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "||") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  psRight = KlelInnerExecute(psRoot->apsChildren[1], psContext);

  if (psLeft == NULL || psRight == NULL)
  {
    KlelFreeResult(psLeft);
    KlelFreeResult(psRight);

    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_BOOLEAN && psRight->iType == KLEL_EXPR_BOOLEAN);

  psResult = KlelCreateBoolean(psLeft->bBoolean || psRight->bBoolean);

   KlelFreeResult(psLeft);
   KlelFreeResult(psRight);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoNot
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoNot(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                        != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]        != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "not") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);

  if (psLeft == NULL)
  {
    KlelFreeResult(psLeft);
    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_BOOLEAN);

  psResult = KlelCreateBoolean(!(psLeft->bBoolean));

  KlelFreeResult(psLeft);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoNegate
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoNegate(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psLeft   = NULL;
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(psRoot                        != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]        != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "neg") == 0);

  psLeft  = KlelInnerExecute(psRoot->apsChildren[0], psContext);

  if (psLeft == NULL)
  {
    KlelFreeResult(psLeft);
    return NULL;
  }

  KLEL_ASSERT(psLeft->iType == KLEL_EXPR_FLOAT || psLeft->iType == KLEL_EXPR_INTEGER);

  if (psLeft->iType == KLEL_EXPR_FLOAT)
  {
    psResult = KlelCreateFloat(-(psLeft->dFloat));
  }

  if (psLeft->iType == KLEL_EXPR_INTEGER)
  {
    psResult = KlelCreateInteger(-(psLeft->llInteger));
  }

  KlelFreeResult(psLeft);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoCall
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoCall(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psActual[KLEL_MAX_FUNC_ARGS] = {NULL};
  KLEL_VALUE *psFunc                       = NULL;
  KLEL_VALUE *psResult                     = NULL;
  size_t     szi                           = 0;
  size_t     szj                           = 0;

  KLEL_ASSERT(psRoot                         != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0]         != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "call") == 0);

  psFunc = KlelInnerExecute(psRoot->apsChildren[0], psContext);
  if (psFunc == NULL)
  {
    return NULL;
  }

  KLEL_ASSERT(psFunc->iType == KLEL_EXPR_FUNCTION_BOOLEAN || psFunc->iType == KLEL_EXPR_FUNCTION_FLOAT ||
              psFunc->iType == KLEL_EXPR_FUNCTION_INTEGER || psFunc->iType == KLEL_EXPR_FUNCTION_STRING);

  if (psRoot->apsChildren[1] != NULL) /* The function has arguments. */
  {
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS; szi++)
    {
      if (psRoot->apsChildren[1]->apsChildren[szi] != NULL)
      {
        psActual[szi] = KlelInnerExecute(psRoot->apsChildren[1]->apsChildren[szi], psContext); /* Get the actual argument. */

        if (psActual[szi] == NULL) /* Execution failed; free the arguments already computed. */
        {
          for (szj = 0; szj < KLEL_MAX_FUNC_ARGS; szj++)
          {
            if (psActual[szj] != NULL && szj != szi)
            {
             KlelFreeResult(psActual[szj]);
            }
          }
          KlelFreeResult(psFunc);
          return NULL; /* The error stack will let the user know what happened. */
        }
      }
    }
  }

  psResult = psFunc->fFunction(psActual, psContext); /* Call the function. */

  for (szi = 0; szi < KLEL_MAX_FUNC_ARGS; szi++) /* Free the actual arguments. */
  {
    if (psActual[szi] != NULL)
    {
     KlelFreeResult(psActual[szi]);
    }
  }
  KlelFreeResult(psFunc); /* Free the function. */

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoString
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoString(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  char       *pcBuffer = NULL;
  char       *pcNew    = NULL;
  size_t     szi       = 0;
  size_t     szLength  = 0;
  size_t     szOff     = 0;
  KLEL_VALUE *psResult = NULL;
  KLEL_VALUE *psValue  = NULL;
  char       *pcString = NULL;

  KLEL_ASSERT(psRoot                           != NULL);
  KLEL_ASSERT(psRoot->acString                 != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "string") == 0);

  for (szi = 0; szi < KLEL_MAX_CHILDREN && psRoot->apsChildren[szi] != NULL; szi++) /* XXX - This is incredibly inefficient. */
  {
    if (strcmp(psRoot->apsChildren[szi]->acName, "fragment") == 0)
    {
      pcNew = realloc(pcBuffer, szOff + psRoot->apsChildren[szi]->szLength);
      if (pcNew == NULL)
      {
        free(pcBuffer);
        return NULL;
      }

      pcBuffer = pcNew;
      memcpy(pcBuffer + szOff, psRoot->apsChildren[szi]->acString, psRoot->apsChildren[szi]->szLength);
      szOff += psRoot->apsChildren[szi]->szLength;
    }
    else if (strcmp(psRoot->apsChildren[szi]->acName, "char") == 0)
    {
      pcNew = realloc(pcBuffer, szOff + 1);
      if (pcNew == NULL)
      {
        free(pcBuffer);
        return NULL;
      }

      pcBuffer = pcNew;
      pcBuffer[szOff] = psRoot->apsChildren[szi]->cChar;
      szOff++;
    }
    else if(strcmp(psRoot->apsChildren[szi]->acName, "var") == 0 || strcmp(psRoot->apsChildren[szi]->acName, "quote") == 0)
    {
      psValue = KlelInnerExecute(psRoot->apsChildren[szi], psContext);

      if (psValue == NULL)
      {
        free(pcBuffer);
        KlelFreeResult(psValue);
        return NULL;
      }

      if (strcmp(psRoot->apsChildren[szi]->acName, "quote") == 0)
      {
        pcString = KlelValueToQuotedString(psValue, &szLength);
      }
      else
      {
        pcString = KlelValueToString(psValue, &szLength);
      }
      KlelFreeResult(psValue);

      if (pcString == NULL)
      {
        free(pcBuffer);
        return NULL;
      }

      pcNew = realloc(pcBuffer, szOff + szLength);
      if (pcNew == NULL)
      {
        free(pcBuffer);
        free(pcString);
        return NULL;
      }

      pcBuffer = pcNew;
      memcpy(pcBuffer + szOff, pcString, szLength);
      free(pcString);
      szOff += szLength;
    }
  }

  psResult = KlelCreateString(szOff, pcBuffer);
  free(pcBuffer);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDoVariable
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelDoVariable(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psValue = NULL;

  KLEL_ASSERT(psRoot                                        != NULL);
  KLEL_ASSERT(psRoot->acName                                != NULL);
  KLEL_ASSERT(psRoot->acString                              != NULL);
  KLEL_ASSERT(KlelGetTypeOfVar(psRoot->acString, psContext) != KLEL_EXPR_UNKNOWN);
  KLEL_ASSERT(strcmp(psRoot->acName, "var") == 0 || strcmp(psRoot->acName, "quote") == 0);

  if (KlelGetTypeOfVar(psRoot->acString, psContext) == KLEL_EXPR_UNKNOWN)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unknown variable", psRoot->acString, NULL);
    return NULL;
  }

  psValue = KlelGetValueOfVar(psRoot->acString, psContext);
  if (psValue == NULL)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "undefined variable", psRoot->acString, NULL);
    return NULL;
  }

  return psValue;
}
