/*-
 ***********************************************************************
 *
 * $Id: typecheck.c,v 1.51 2019/07/31 15:59:27 klm Exp $
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
#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * Type checking rules.
 *
 * This table maps node types to either a node-specific callback, or,
 * for unary and binary operators, one or two operand types and a result
 * type. Finally, an error message is provided.
 *
 * The typechecking algorithm is simple:
 * If there is a callback, call it and return.
 * If there's an error message, report the error and return.
 * Otherwise, check and see if its operands match the specified types,
 * and, if so, return the specified result type.
 *
 * Obviously, order matters: error entries must be lower than all other
 * rules for that node type.
 *
 ***********************************************************************
 */
static KLEL_TYPE_DESC gasKlelTypeRules[] =
{
  {KLEL_NODE_AND,           NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_AND_AND,       NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_BANG,          NULL, KLEL_TYPE_BOOLEAN, 0,                 KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_CARET,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_DOT,           NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  NULL},
  {KLEL_NODE_EQ_EQ,         NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_EQ_EQ,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_EQ_EQ,         NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_EQ_EQ,         NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_FRAGMENT,      NULL, 0,                 0,                 KLEL_TYPE_STRING,  NULL},
  {KLEL_NODE_GT,            NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GT,            NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GT,            NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GT,            NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GTE,           NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GTE,           NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GTE,           NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GTE,           NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_GT_GT,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_INTEGER,       NULL, 0,                 0,                 KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_LT,            NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LT,            NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LT,            NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LT,            NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LTE,           NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LTE,           NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LTE,           NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LTE,           NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_LT_LT,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_MINUS,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_MINUS,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_MINUS,         NULL, KLEL_TYPE_REAL,    KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_MINUS,         NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_NE,            NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_NE,            NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_NE,            NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_NE,            NULL, KLEL_TYPE_STRING,  KLEL_TYPE_STRING,  KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_NEGATE,        NULL, KLEL_TYPE_INT64,   0,                 KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_NEGATE,        NULL, KLEL_TYPE_REAL,    0,                 KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_PERCENT,       NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_PIPE,          NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_PIPE_PIPE,     NULL, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, KLEL_TYPE_BOOLEAN, NULL},
  {KLEL_NODE_PLUS,          NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_PLUS,          NULL, KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_PLUS,          NULL, KLEL_TYPE_REAL,    KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_PLUS,          NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_REAL,          NULL, 0,                 0,                 KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_SLASH,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_SLASH,         NULL, KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_SLASH,         NULL, KLEL_TYPE_REAL,    KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_SLASH,         NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_STAR,          NULL, KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   KLEL_TYPE_INT64,   NULL},
  {KLEL_NODE_STAR,          NULL, KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_STAR,          NULL, KLEL_TYPE_REAL,    KLEL_TYPE_INT64,   KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_STAR,          NULL, KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    KLEL_TYPE_REAL,    NULL},
  {KLEL_NODE_TILDE,         NULL, KLEL_TYPE_INT64,   0,                 KLEL_TYPE_INT64,   NULL},

  {KLEL_NODE_AND,           NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'&' is only defined for integer operands"},
  {KLEL_NODE_AND_AND,       NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'&&' is only defined for boolean operands"},
  {KLEL_NODE_BANG,          NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'!' is only defined for boolean operands"},
  {KLEL_NODE_CARET,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'^' is only defined for integer operands"},
  {KLEL_NODE_DOT,           NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'.' is only defined for string operands"},
  {KLEL_NODE_EQ_EQ,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'==' must have operands of equal types"},
  {KLEL_NODE_GT,            NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'>' must have operands of equal types"},
  {KLEL_NODE_GTE,           NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'>=' must have operands of equal types"},
  {KLEL_NODE_GT_GT,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'<<' is only defined for integer operands"},
  {KLEL_NODE_LT,            NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'<' must have operands of equal types"},
  {KLEL_NODE_LTE,           NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'<=' must have operands of equal types"},
  {KLEL_NODE_LT_LT,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'>>' is only defined for integer operands"},
  {KLEL_NODE_MINUS,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'-' is only defined for numeric operands"},
  {KLEL_NODE_NE,            NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'!=' must have operands of equal types"},
  {KLEL_NODE_NEGATE,        NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'-' is only defined for numeric operands"},
  {KLEL_NODE_PERCENT,       NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'%' is only defined for integer operands"},
  {KLEL_NODE_PIPE,          NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'|' is only defined for integer operands"},
  {KLEL_NODE_PIPE_PIPE,     NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'||' is only defined for boolean operands"},
  {KLEL_NODE_PLUS,          NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'+' is only defined for numeric operands"},
  {KLEL_NODE_SLASH,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'/' is only defined for numeric operands"},
  {KLEL_NODE_STAR,          NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'*' is only defined for numeric operands"},
  {KLEL_NODE_TILDE,         NULL, 0,                 0,                 KLEL_TYPE_UNKNOWN, "'~' is only defined for integer operands"},

  {KLEL_NODE_CALL,            KlelTypeCheckCall},
  {KLEL_NODE_CONDITIONAL,     KlelTypeCheckConditional},
  {KLEL_NODE_DESIGNATOR,      KlelTypeCheckDesignator},
  {KLEL_NODE_EXPRESSION,      KlelTypeCheckExpression},
  {KLEL_NODE_GUARDED_COMMAND, KlelTypeCheckGuardedCommand},
  {KLEL_NODE_INTERP,          KlelTypeCheckInterp},
  {KLEL_NODE_LET,             KlelTypeCheckLet},
  {KLEL_NODE_QUOTED_INTERP,   KlelTypeCheckInterp},
  {KLEL_NODE_LIKE,            KlelTypeCheckLike},
  {KLEL_NODE_UNLIKE,          KlelTypeCheckLike}
};


/*-
 ***********************************************************************
 *
 * KlelTypeCheck
 * This implements the type checking algorithm using the rules table
 * defined above.
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheck(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  size_t szi = 0;

  for (szi = 0; szi < sizeof(gasKlelTypeRules) / sizeof(KLEL_TYPE_DESC); szi++)
  {
    if (gasKlelTypeRules[szi].iType == psNode->iType)
    {
      if (gasKlelTypeRules[szi].pfFunction != NULL)
      {
        return gasKlelTypeRules[szi].pfFunction(psNode, psContext);
      }
      else if (gasKlelTypeRules[szi].pcError != NULL)
      {
        KlelReportError(psContext, "%s", gasKlelTypeRules[szi].pcError, NULL);
        return KLEL_TYPE_UNKNOWN;
      }
      else
      {
        if
        (
             gasKlelTypeRules[szi].iOperand1 == 0
          || gasKlelTypeRules[szi].iOperand1 == KlelTypeCheck(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext)
        )
        {
          if
          (
               gasKlelTypeRules[szi].iOperand2 == 0
            || gasKlelTypeRules[szi].iOperand2 == KlelTypeCheck(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext)
          )
          {
            return gasKlelTypeRules[szi].iResult;
          }
        }
      }
    }
  }

  KLEL_ASSERT(0);
  return KLEL_TYPE_UNKNOWN;
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckCall
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckCall(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_EXPR_TYPE iFunctionType   = KlelInnerGetTypeOfVar(psContext, psRoot->acFragment, psContext->pvData);
  unsigned int   uiArgumentCount = KLEL_GET_ARGUMENT_COUNT(iFunctionType);
  unsigned int   uiActualCount   = 0;

  KLEL_ASSERT(psRoot          != NULL);
  KLEL_ASSERT(psContext       != NULL);
  KLEL_ASSERT(KLEL_IS_FUNCTION(iFunctionType));

  if (iFunctionType == KLEL_TYPE_UNKNOWN)
  {
    KlelReportError(psContext, "unknown function '%s'", psRoot->acFragment, NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  for (uiActualCount = 0; uiActualCount < KLEL_MAX_FUNC_ARGS && psRoot->apsChildren[uiActualCount] != NULL; uiActualCount++)
    ; /* Update uiActualCount with the count of actual arguments. */

  if (uiActualCount != uiArgumentCount)
  {
    KlelReportError(psContext, "incorrect number of arguments to function '%s'", psRoot->acFragment, NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  for (uiArgumentCount = 0; uiArgumentCount < KLEL_MAX_FUNC_ARGS; uiArgumentCount++)
  {
    if (KLEL_GET_ARGUMENT(iFunctionType, uiArgumentCount) != KLEL_TYPE_UNKNOWN)
    {
      if
      (
           psRoot->apsChildren[uiArgumentCount] == NULL
        || KLEL_GET_ARGUMENT(iFunctionType, uiArgumentCount) != KlelTypeCheck(psRoot->apsChildren[uiArgumentCount], psContext)
      )
      {
        KlelReportError(psContext, "invalid arguments to function '%s'", psRoot->acFragment, NULL);
        return KLEL_TYPE_UNKNOWN;
      }
    }
  }

  return KLEL_GET_RETURN_TYPE(iFunctionType);
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckConditional
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckConditional(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_EXPR_TYPE iPredicateType = KlelTypeCheck(psNode->apsChildren[KLEL_PREDICATE_INDEX], psContext);
  KLEL_EXPR_TYPE iIfFalseType   = KlelTypeCheck(psNode->apsChildren[KLEL_IFFALSE_INDEX],   psContext);
  KLEL_EXPR_TYPE iIfTrueType    = KlelTypeCheck(psNode->apsChildren[KLEL_IFTRUE_INDEX],    psContext);

  if (iPredicateType != KLEL_TYPE_BOOLEAN)
  {
    KlelReportError(psContext, "conditional predicates must be of boolean type", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  if (iIfFalseType != iIfTrueType)
  {
    KlelReportError(psContext, "both branches of conditional expressions must be of the same type", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  return iIfFalseType;
}


/*-
 ***********************************************************************
 *
 * KlelTypeDesignator
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckDesignator(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_EXPR_TYPE iType = KLEL_TYPE_UNKNOWN;

  KLEL_ASSERT(psNode    != NULL);
  KLEL_ASSERT(psContext != NULL);

  /*-
   *********************************************************************
   *
   * Check to see if its closure is negative. If so, check with the host
   * to get the type of the variable. Otherwise, get the type from the
   * closure.
   *
   *********************************************************************
   */

  iType = psNode->iClosure < 0 ? KlelInnerGetTypeOfVar(psContext, psNode->acFragment, psContext->pvData)
                               : psContext->psClosures[psNode->iClosure].iType;

  if (iType == KLEL_TYPE_UNKNOWN)
  {
    KlelReportError(psContext, "unknown variable '%s'", psNode->acFragment, NULL);
  }
  else if (KLEL_IS_FUNCTION(iType))
  {
    KlelReportError(psContext, "bare function '%s' in expression", psNode->acFragment, NULL);
    iType = KLEL_TYPE_UNKNOWN;
  }

  return iType;
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckExpression
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckExpression(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psNode    != NULL);
  KLEL_ASSERT(psContext != NULL);

  return KlelTypeCheck(psNode->apsChildren[KLEL_EXPRESSION_INDEX], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckGuardedCommand
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckGuardedCommand(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  size_t szi = 0;

  KLEL_ASSERT(psNode    != NULL);
  KLEL_ASSERT(psContext != NULL);

  psNode = psNode->apsChildren[KLEL_EXPRESSION_INDEX];

  if
  (
       !KlelIsConstantString(psNode->apsChildren[KLEL_INTERPRETER_INDEX])
    || !KlelIsConstantString(psNode->apsChildren[KLEL_COMMAND_INDEX])
  )
  {
    KlelReportError(psContext, "interpreter and command arguments to the eval function must be constant strings", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  if (KlelConstantStringLength(psNode->apsChildren[KLEL_INTERPRETER_INDEX]) >= KLEL_MAX_NAME)
  {
    KlelReportError(psContext, "interpreter argument to the eval function is too long", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  if (KlelConstantStringLength(psNode->apsChildren[KLEL_COMMAND_INDEX]) >= KLEL_MAX_NAME)
  {
    KlelReportError(psContext, "command argument to the eval function is too long", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  for (szi = 0; szi < KLEL_MAX_FUNC_ARGS; szi++)
  {
    if (psNode->apsChildren[szi] != NULL)
    {
      if (KlelTypeCheck(psNode->apsChildren[szi], psContext) == KLEL_TYPE_UNKNOWN)
      {
        return KLEL_TYPE_UNKNOWN;
      }
    }
  }

  if (KlelTypeCheck(psNode->apsChildren[KLEL_PREDICATE_INDEX], psContext) != KLEL_TYPE_BOOLEAN)
  {
    KlelReportError(psContext, "guarded command predicates must be of boolean type", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  return KLEL_TYPE_BOOLEAN;
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckLet
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckLet(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psNode    != NULL);
  KLEL_ASSERT(psContext != NULL);

  psContext->psClosures[psNode->iClosure].iType = KlelTypeCheck(psNode->apsChildren[KLEL_DEFINITION_INDEX], psContext);
  if (psContext->psClosures[psNode->iClosure].iType == KLEL_TYPE_UNKNOWN)
  {
    return KLEL_TYPE_UNKNOWN;
  }

  return KlelTypeCheck(psNode->apsChildren[KLEL_EXPRESSION_INDEX], psContext);
}


/*-
 ***********************************************************************
 *
 * KlelTypeInterp
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckInterp(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psNode    != NULL);
  KLEL_ASSERT(psContext != NULL);

  if (KlelTypeCheckDesignator(psNode, psContext) != KLEL_TYPE_UNKNOWN)
  {
    return KLEL_TYPE_STRING;
  }

  return KLEL_TYPE_UNKNOWN;
}


/*-
 ***********************************************************************
 *
 * KlelTypeLike
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckLike(KLEL_NODE *psNode, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psValue      = NULL;
  int        iOffset       = 0;
  const char *pcError      = NULL;
  pcre       *psExpression = NULL;

  KLEL_ASSERT(psNode    != NULL);
  KLEL_ASSERT(psContext != NULL);

  if
  (
       KlelTypeCheck(psNode->apsChildren[KLEL_OPERAND1_INDEX], psContext) != KLEL_TYPE_STRING
    || KlelTypeCheck(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext) != KLEL_TYPE_STRING
  )
  {
    KlelReportError(psContext, "'%s' is only defined for string operands", psNode->iType == KLEL_NODE_LIKE ? "=~" : "!~", NULL);
    return KLEL_TYPE_UNKNOWN;
  }

  /*-
   *********************************************************************
   *
   * If the regular expression is a constant string, we can check it at
   * compile time to see if it's valid. If it's not a constant string
   * (i.e. it contains interpolations or is a variable), we have to wait
   * until run time.
   *
   *********************************************************************
   */

  if (KlelIsConstantString(psNode->apsChildren[KLEL_OPERAND2_INDEX]))
  {
    psValue = KlelInnerExecute(psNode->apsChildren[KLEL_OPERAND2_INDEX], psContext);
    if (psValue == NULL)
    {
      KlelReportError(psContext, "out of memory", NULL);
      return KLEL_TYPE_UNKNOWN;
    }

    psExpression = pcre_compile(SteelStringToCString(psValue->psString), 0, &pcError, &iOffset, NULL);
    KlelFreeResult(psValue);
    if (psExpression == NULL)
    {
      KlelReportError(psContext, "regular expression is invalid: %s", pcError, NULL);
      return KLEL_TYPE_UNKNOWN;
    }
    pcre_free(psExpression);
  }

  return KLEL_TYPE_BOOLEAN;
}
