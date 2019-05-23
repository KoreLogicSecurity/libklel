/*-
 ***********************************************************************
 *
 * $Id: parser.c,v 1.51 2012/04/23 16:40:22 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#define _BSD_SOURCE /* For v?snprintf. */
#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * KlelPopError
 *
 ***********************************************************************
 */
void
KlelPopError(KLEL_CONTEXT *psContext)
{
  KLEL_ERROR *psError = NULL;

  if (psContext != NULL)
  {
    if (psContext->psErrorList != NULL)
    {
      psError = psContext->psErrorList;
      psContext->psErrorList = psError->psNext;
      psContext->psNextError = NULL;
      free(psError);
    }
  }
}


/*-
 ***********************************************************************
 *
 * KlelCreateNode
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCreateNode(const char *pcName, const char *pcInput, const char *pcRemainder, size_t szCount, ...)
{
  size_t    szi       = 0;
  KLEL_NODE *psChild  = NULL;
  KLEL_NODE *psResult = calloc(1, sizeof(KLEL_NODE));
  va_list   vlArgs;

  KLEL_ASSERT(pcName       != NULL);
  KLEL_ASSERT(pcInput      != NULL);
  KLEL_ASSERT(pcRemainder  != NULL);
  KLEL_ASSERT(pcRemainder  >= pcInput);
  KLEL_ASSERT(szCount       < KLEL_MAX_CHILDREN);

  if (psResult != NULL)
  {
    psResult->iType       = szCount == 0 ? KLEL_NODE_TERMINAL : KLEL_NODE_NONTERMINAL;
    psResult->pcInput     = pcInput;
    psResult->pcRemainder = pcRemainder;
    psResult->szLength    = 0;
    strncpy(psResult->acName, pcName, KLEL_MAX_NAME);

    va_start(vlArgs, szCount);

    for (szi = 0; szi < szCount && szi < KLEL_MAX_CHILDREN; szi++)
    {
      psChild = va_arg(vlArgs, KLEL_NODE *);
      if (psChild != NULL)
      {
        psResult->apsChildren[szi]  = psChild;
        psResult->szLength         += psChild->szLength;
      }
    }

    va_end(vlArgs);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelParse
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelParse(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psName   = NULL;
  KLEL_NODE *psColon  = NULL;
  KLEL_NODE *psResult = NULL;

  KLEL_ASSERT(pcInput   != NULL);

  psName = KlelTerminal("^[A-Za-z_][A-Za-z0-9_]*\\s*:", "name", KlelSpace(pcInput));
  if (psName != NULL && psName->iType != KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psName);
    psName = KlelTerminal("^[A-Za-z_][A-Za-z0-9_]*", "name", KlelSpace(pcInput));
    KLEL_ASSERT(psName != NULL && psName->iType != KLEL_NODE_INVALID);
    if (psName == NULL || psName->iType == KLEL_NODE_INVALID)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid name", pcInput, NULL);
      KlelFreeNode(&psName);
      return NULL;
    }

    psColon = KlelTerminal("^:", "colon", KlelSpace(psName->pcRemainder));
    KLEL_ASSERT(psColon != NULL && psColon->iType != KLEL_NODE_INVALID);
    if (psColon == NULL || psColon->iType == KLEL_NODE_INVALID)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid name", pcInput, NULL);
      KlelFreeNode(&psName);
      KlelFreeNode(&psColon);
      return NULL;
    }
    pcInput = KlelSpace(psColon->pcRemainder);
    KlelFreeNode(&psColon);
  }
  else
  {
    KlelFreeNode(&psName);
    psName = NULL;
  }

  psResult = KlelGuardedCommand(pcInput, psContext);
  if (psResult == NULL || psResult->iType == KLEL_NODE_INVALID)
  {
    if (KlelSpace(pcInput)[0] == 'i' && KlelSpace(pcInput)[1] == 'f') /* Probably a guarded command. */
    {
      if (psResult != NULL)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, psResult->acError, pcInput, NULL);
      }
      KlelFreeNode(&psName);
      KlelFreeNode(&psResult);
      return NULL;
    }
    KlelFreeNode(&psResult);

    KlelClearErrors(psContext);
    KlelFreeNode(&psResult);

    psResult = KlelExpression(pcInput, psContext);
    if (psResult == NULL || psResult->iType == KLEL_NODE_INVALID)
    {
      KlelFreeNode(&psName);
      KlelFreeNode(&psResult);
      if (psResult != NULL)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, psResult->acError, pcInput, NULL);
      }
      return NULL;
    }
  }

  if (KlelSpace(psResult->pcRemainder)[0] != 0)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "extra input at end of expression", KlelSpace(psResult->pcRemainder), NULL);
    KlelFreeNode(&psName);
    KlelFreeNode(&psResult);
    return NULL;
  }

  return KlelCreateNode("root", psResult->pcInput, psResult->pcRemainder, 2, psName, psResult);
}


/*-
 ***********************************************************************
 *
 * KlelSpace
 *
 ***********************************************************************
 */
const char *
KlelSpace(const char *pcInput)
{
  KLEL_ASSERT(pcInput != NULL);

  while (pcInput[0] != 0 && isspace(pcInput[0]))
  {
    pcInput++;
  }

  return pcInput;
}


/*-
 ***********************************************************************
 *
 * KlelTerminal
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelTerminal(const char *pcRegex, const char *pcName, const char *pcInput)
{
  const char *pcErr     = NULL;
  int        iFlags     = PCRE_ANCHORED | PCRE_DOLLAR_ENDONLY | PCRE_DOTALL;
  size_t     szLen      = strlen(pcInput);
  int        iOff       = 0;
  pcre       *psPattern = pcre_compile(pcRegex, iFlags, &pcErr, &iOff, NULL);
  KLEL_NODE  *psResult  = NULL;
  int        aiVec[6]   = {0};

  KLEL_ASSERT(pcRegex   != NULL);
  KLEL_ASSERT(pcName    != NULL);
  KLEL_ASSERT(psPattern != NULL);
  KLEL_ASSERT(pcInput   != NULL);

  psResult = calloc(1, sizeof(KLEL_NODE));
  if (psResult != NULL)
  {
    psResult->iType       = KLEL_NODE_INVALID;
    psResult->pcInput     = pcInput;
    psResult->pcRemainder = pcInput;
    snprintf(psResult->acName,  KLEL_MAX_NAME,  "%s", pcName);
    snprintf(psResult->acError, KLEL_MAX_ERROR, "expected %s", pcName);

    if (psPattern != NULL)
    {
      if (pcre_exec(psPattern, NULL, pcInput, szLen, 0, 0, aiVec, 6) > 0)
      {
        free(psResult);
        psResult = calloc(1, sizeof(KLEL_NODE) + (aiVec[1] - aiVec[0]) + 1);

        if (psResult != NULL)
        {
          psResult->iType       = KLEL_NODE_TERMINAL;
          psResult->pcInput     = pcInput;
          psResult->szLength    = aiVec[1] - aiVec[0];
          psResult->pcRemainder = pcInput + psResult->szLength;
          strncpy(psResult->acName, pcName, KLEL_MAX_NAME);
          strncpy(psResult->acString, pcInput, aiVec[1] - aiVec[0]);
        }
      }

      pcre_free(psPattern);
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelAddOperator = '.' | '+' | '-' | '||' | '&' | '^' | '|'
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelAddOperator(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psResult = NULL;

  KLEL_ASSERT(pcInput != NULL);

  psResult = KlelTerminal("^([.]|[+]|[-]|[|][|]|[&]|[|]|\\^)", "op", KlelSpace(pcInput));
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    strncpy(psResult->acName, psResult->acString, KLEL_MAX_NAME);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * Character =  '\' ('\' | '"' | 'n' | 'r' | 'x' hexdig hexdig | '$')
 * Character =/ '%{' designator '}' | '%(' designator ')'
 * Character =/ <any character other than '\', '"', 'n', 'r', '%' or control>
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCharacter(const char *pcInput, KLEL_CONTEXT *psContext)
{
  long      lChar     = 0;
  char      *pcEnd    = NULL;
  KLEL_NODE *psResult = NULL;
  KLEL_NODE *psSigil  = NULL;

  KLEL_ASSERT(pcInput != NULL);

  psResult = KlelTerminal("^\\x5c\\x5c", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    psResult->cChar = '\\';

    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^\\x5c\"", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    psResult->cChar = '"';
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^\\x5cn", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    psResult->cChar = '\n';
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^\\x5cr", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    psResult->cChar = '\r';
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^\\x5c%", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    psResult->cChar = '%';
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^\\x5cx..", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    errno = 0;
    lChar = strtol(psResult->acString + 2, &pcEnd, 16);
    if (errno != 0 || pcEnd[0] != '\0')
    {
      KlelFreeNode(&psResult);
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid hex escape sequence", pcInput, NULL);
      return NULL;
    }

    psResult->cChar = (char)lChar;
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^\\x5c.", "char", pcInput);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid escape sequence", pcInput, NULL);
    KlelFreeNode(&psResult);
    return NULL;
  }
  KlelFreeNode(&psResult);

  psSigil = KlelTerminal("^%[{]", "sigil", pcInput);
  if (psSigil != NULL && psSigil->iType != KLEL_NODE_INVALID)
  {
    psResult = KlelDesignator(KlelSpace(psSigil->pcRemainder), psContext);
    KlelFreeNode(&psSigil);

    if (psResult == NULL || psResult->iType == KLEL_NODE_INVALID)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "incomplete interpolation", pcInput, NULL);
      return psResult;
    }

    psSigil = KlelTerminal("^[}]", "sigil", KlelSpace(psResult->pcRemainder));
    if (psSigil == NULL || psSigil->iType == KLEL_NODE_INVALID)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unterminated interpolation", pcInput, NULL);
      KlelFreeNode(&psResult);
      return psSigil;
    }
    psResult->pcRemainder = psSigil->pcRemainder;
    KlelFreeNode(&psSigil);

    return psResult;
  }
  KlelFreeNode(&psSigil);

  psSigil = KlelTerminal("^%[(]", "sigil", pcInput);
  if (psSigil != NULL && psSigil->iType != KLEL_NODE_INVALID)
  {
    psResult = KlelQuotedDesignator(KlelSpace(psSigil->pcRemainder), psContext);
    KlelFreeNode(&psSigil);
    if (psResult == NULL || psResult->iType == KLEL_NODE_INVALID)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "incomplete interpolation", pcInput, NULL);
      return psResult;
    }

    psSigil = KlelTerminal("^[)]", "sigil", KlelSpace(psResult->pcRemainder));
    if (psSigil == NULL || psSigil->iType == KLEL_NODE_INVALID)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unterminated interpolation", pcInput, NULL);
      KlelFreeNode(&psResult);
      return psSigil;
    }
    psResult->pcRemainder = psSigil->pcRemainder;
    KlelFreeNode(&psSigil);

    return psResult;
  }
  KlelFreeNode(&psSigil);

  return KlelTerminal("^([^\\x00-\\x1f\\x22\\x25\\x5c]|%[^\\x28\\x7b])+", "fragment", pcInput);
}


/*-
 ***********************************************************************
 *
 * Conditional = Factor '?' Expression ':' Expression
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelConditional(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psColon       = NULL;
  KLEL_NODE *psFalse       = NULL;
  KLEL_NODE *psPredicate   = NULL;
  KLEL_NODE *psQuestion    = NULL;
  KLEL_NODE *psTrue        = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Factor */
  psPredicate = KlelFactor(pcInput, psContext);
  if (psPredicate == NULL || psPredicate->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psPredicate);
    return NULL;
  }

  /* '?' */
  psQuestion = KlelTerminal("^[?]", "?", KlelSpace(psPredicate->pcRemainder));
  if (psQuestion == NULL || psQuestion->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psPredicate);

    return psQuestion;
  }

  pcInput = KlelSpace(psQuestion->pcRemainder);
  KlelFreeNode(&psQuestion);

  /* Expression */
  psTrue = KlelExpression(pcInput, psContext);
  if (psTrue == NULL || psTrue->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psPredicate);

    return psTrue;
  }

  /* ':' */
  psColon = KlelTerminal("^:", ":", KlelSpace(psTrue->pcRemainder));
  if (psColon == NULL || psColon->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psTrue);
    KlelFreeNode(&psPredicate);

    return psColon;
  }

  pcInput = KlelSpace(psColon->pcRemainder);
  KlelFreeNode(&psColon);

  /* Expression */
  psFalse = KlelExpression(pcInput, psContext);
  if (psFalse == NULL || psFalse->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psTrue);
    KlelFreeNode(&psPredicate);

    return psFalse;
  }

  /* Conditional */
  return KlelCreateNode("cond", psPredicate->pcInput, psFalse->pcRemainder, 3, psPredicate, psTrue, psFalse);
}


/*-
 ***********************************************************************
 *
 * Call = Designator '(' [ExprList] ')'
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCall(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psName   = NULL;
  KLEL_NODE *psParams = NULL;
  KLEL_NODE *psParen  = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Designator */
  psName = KlelDesignator(pcInput, psContext);
  if (psName == NULL || psName->iType == KLEL_NODE_INVALID)
  {
    return psName;
  }

  /* '(' */
  psParen = KlelTerminal("^[(]", "(", KlelSpace(psName->pcRemainder));
  if (psParen == NULL || psParen->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psName);
    return psParen;
  }
  pcInput = KlelSpace(psParen->pcRemainder);
  KlelFreeNode(&psParen);

  /* [ExprList] */
  psParams = KlelExprList(pcInput, psContext);
  if (psParams == NULL || psParams->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psParams);
  }
  else
  {
    pcInput = KlelSpace(psParams->pcRemainder);
  }

  /* ')' */
  psParen = KlelTerminal("^[)]", ")", pcInput);
  if (psParen == NULL || psParen->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psName);
    KlelFreeNode(&psParams);
    return psParen;
  }
  pcInput = KlelSpace(psParen->pcRemainder);
  KlelFreeNode(&psParen);

  /* Call */
  return KlelCreateNode("call", psName->pcInput, pcInput, 2, psName, psParams);
}


/*-
 ***********************************************************************
 *
 * Designator = (letter | '_') {letter | number | '_' | ':' }
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelDesignator(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psResult = KlelTerminal("^[A-Za-z_][A-Za-z0-9_]*", "var", KlelSpace(pcInput));

  if
  (
       strcmp(psResult->acString, "if")   == 0
    || strcmp(psResult->acString, "then") == 0
    || strcmp(psResult->acString, "eval") == 0
    || strcmp(psResult->acString, "pass") == 0
    || strcmp(psResult->acString, "fail") == 0
  )
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unexpected keyword", pcInput, NULL);
    KlelFreeNode(&psResult);
    return NULL;
  }

  return psResult;
}

KLEL_NODE *
KlelQuotedDesignator(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(pcInput != NULL);

  return KlelTerminal("^[A-Za-z_][A-Za-z0-9_:]*", "quote", KlelSpace(pcInput));
}


/*-
 ***********************************************************************
 *
 * Expression =  Conditional
 * Expression =/ SimpleExpression [Relation SimpleExpression]
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelExpression(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psLeft   = NULL;
  KLEL_NODE *psOp     = NULL;
  KLEL_NODE *psResult = NULL;
  KLEL_NODE *psRight  = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Expression = Conditional */
  psLeft = KlelConditional(pcInput, psContext);
  if (psLeft != NULL && psLeft->iType != KLEL_NODE_INVALID)
  {
    return psLeft;
  }
  KlelPopError(psContext);
  KlelFreeNode(&psLeft);

  /* Expression =/ SimpleExpression [Relation SimpleExpression] */
  psLeft = KlelSimpleExpression(pcInput, psContext);
  if (psLeft == NULL || psLeft->iType == KLEL_NODE_INVALID)
  {
    return psLeft;
  }

  /* [Relation SimpleExpression] */
  psOp = KlelRelation(KlelSpace(psLeft->pcRemainder), psContext);
  if (psOp == NULL || psOp->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psOp);
    return psLeft;
  }

  psRight = KlelSimpleExpression(KlelSpace(psOp->pcRemainder), psContext);
  if (psRight == NULL || psRight->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psLeft);
    KlelFreeNode(&psOp);

    return psRight;
  }

  /* Expression */
  psResult = KlelCreateNode(psOp->acName, pcInput, psRight->pcRemainder, 2, psLeft, psRight);
  KlelFreeNode(&psOp);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * ExprList = Expression {',' Expression}
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelExprList(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE         *psComma  = NULL;
  size_t            szi       = 1;
  KLEL_NODE         *psLeft   = NULL;
  KLEL_NODE         *psResult = NULL;
  KLEL_NODE         *psRight  = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Expression */
  psLeft = KlelExpression(pcInput, psContext);
  if (psLeft == NULL || psLeft->iType == KLEL_NODE_INVALID)
  {
    return psLeft;
  }

  /* Build the result. */
  psResult = KlelCreateNode("args", psLeft->pcInput, psLeft->pcRemainder, 1, psLeft);
  if (psResult == NULL)
  {
    KlelFreeNode(&psLeft);
    return NULL;
  }

  /* {',' Expression} */
  psComma = KlelTerminal("^,", ",", KlelSpace(psLeft->pcRemainder));
  if (psComma == NULL || psComma->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psComma);
    return psResult;
  }

  while (psComma != NULL && psComma->iType != KLEL_NODE_INVALID)
  {
    psRight = KlelExpression(KlelSpace(psComma->pcRemainder), psContext);
    if (psRight == NULL || psRight->iType == KLEL_NODE_INVALID)
    {
      KlelFreeNode(&psComma);
      KlelFreeNode(&psResult);

      return psRight;
    }

    psResult->apsChildren[szi++] = psRight;
    psResult->pcRemainder = psRight->pcRemainder;
    if (szi >= KLEL_MAX_FUNC_ARGS)
    {
      KlelFreeNode(&psComma);
      KlelFreeNode(&psResult);
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "too many arguments to function or too many expressions in execution list", pcInput, NULL);
      return NULL;
    }

    KlelFreeNode(&psComma);
    psComma = KlelTerminal("^,", ",", KlelSpace(psRight->pcRemainder));
  }

  if (psComma != NULL)
  {
    KLEL_ASSERT(psComma->iType == KLEL_NODE_INVALID);
    KlelFreeNode(&psComma);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * Factor =   Number | String | Call | Designator
 * Factor =/  '(' Expression ')' | '!' Factor | '-' Factor
 * Factor =/  '~' Factor
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelFactor(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psBang   = NULL;
  KLEL_NODE *psMinus  = NULL;
  KLEL_NODE *psParen  = NULL;
  KLEL_NODE *psResult = NULL;
  KLEL_NODE *psTilde  = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Number */
  psResult = KlelNumber(pcInput, psContext);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    return psResult;
  }
  KlelFreeNode(&psResult);

  /* String */
  psResult = KlelString(pcInput, psContext);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    return psResult;
  }
  KlelFreeNode(&psResult);

  /* Call */
  psResult = KlelCall(pcInput, psContext);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    return psResult;
  }
  KlelFreeNode(&psResult);

  /* Designator */
  psResult = KlelDesignator(pcInput, psContext);
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    return psResult;
  }
  KlelFreeNode(&psResult);

  /* '(' Expression ')' */
  psParen = KlelTerminal("^[(]", "(", pcInput);
  if (psParen != NULL && psParen->iType != KLEL_NODE_INVALID)
  {
    pcInput = KlelSpace(psParen->pcRemainder);
    KlelFreeNode(&psParen);

    psResult = KlelExpression(pcInput, psContext);
    if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
    {
      psParen = KlelTerminal("^[)]", ")", KlelSpace(psResult->pcRemainder));
      if (psParen == NULL || psParen->iType == KLEL_NODE_INVALID)
      {
        KlelFreeNode(&psResult);
        return psParen;
      }

      psResult->pcRemainder = psParen->pcRemainder;
      KlelFreeNode(&psParen);
      return psResult;
    }
  }
  KlelFreeNode(&psParen);

  /* '!' Factor */
  psBang = KlelTerminal("^[!]", "!", pcInput);
  if (psBang != NULL && psBang->iType != KLEL_NODE_INVALID)
  {
    psResult = KlelFactor(KlelSpace(psBang->pcRemainder), psContext);
    KlelFreeNode(&psBang);

    if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
    {
      return KlelCreateNode("not", psResult->pcInput, psResult->pcRemainder, 1, psResult);
    }

    return psResult;
  }
  KlelFreeNode(&psBang);

  /* '-' Factor */
  psMinus = KlelTerminal("^[-]", "-", pcInput);
  if (psMinus != NULL && psMinus->iType != KLEL_NODE_INVALID)
  {
    psResult = KlelFactor(KlelSpace(psMinus->pcRemainder), psContext);
    KlelFreeNode(&psMinus);

    if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
    {
      return KlelCreateNode("neg", psResult->pcInput, psResult->pcRemainder, 1, psResult);
    }

    return psResult;
  }
  KlelFreeNode(&psMinus);

  /* '~' Factor */
  psTilde = KlelTerminal("^[~]", "bnot", pcInput);
  if (psTilde != NULL && psTilde->iType != KLEL_NODE_INVALID)
  {
    psResult = KlelFactor(KlelSpace(psTilde->pcRemainder), psContext);
    KlelFreeNode(&psTilde);

    if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
    {
      return KlelCreateNode("bnot", psResult->pcInput, psResult->pcRemainder, 1, psResult);
    }

    return psResult;
  }
  KlelFreeNode(&psTilde);

  KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "expected one of number, string, call, designator, or expression", pcInput, NULL);
  return NULL;
}

/*-
 ***********************************************************************
 *
 * GuardedCommand =   '{' Expression '}' '->' '[' ExprList ']'
 * GuardedCommand =/  '{' Expression '}' '->' String
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelGuardedCommand(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psExpr     = NULL;
  KLEL_NODE *psExprList = NULL;
  KLEL_NODE *psIf       = NULL;
  KLEL_NODE *psThen     = NULL;
  KLEL_NODE *psEval     = NULL;
  KLEL_NODE *psClose    = NULL;
  KLEL_NODE *psResult   = NULL;
  KLEL_NODE *psString   = NULL;
  KLEL_NODE *psCodes    = NULL;
  KLEL_NODE *psSigil    = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* 'if' */
  psIf = KlelTerminal("^if\\s*[(]", "if", KlelSpace(pcInput));
  if (psIf == NULL || psIf->iType == KLEL_NODE_INVALID)
  {
    return psIf;
  }
  pcInput = KlelSpace(psIf->pcRemainder);
  KlelFreeNode(&psIf);

  /* Expression */
  psExpr = KlelExpression(pcInput, psContext);
  if (psExpr == NULL || psExpr->iType == KLEL_NODE_INVALID)
  {
    return psExpr;
  }

  /* 'then' */
  psThen = KlelTerminal("^[)]\\s*then[^A-Za-z_]", "then", KlelSpace(psExpr->pcRemainder));
  if (psThen == NULL || psThen->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psExpr);
    KlelFreeNode(&psThen);
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unterminated guard expression", pcInput, NULL);
    return NULL;
  }
  pcInput = KlelSpace(psThen->pcRemainder);
  KlelFreeNode(&psThen);

  /* 'eval(' */
  psEval = KlelTerminal("^eval\\s*[(]", "eval", pcInput);
  if (psEval == NULL || psEval->iType == KLEL_NODE_INVALID)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "missing guarded command", pcInput, NULL);
    KlelFreeNode(&psEval);
    KlelFreeNode(&psExpr);
    return NULL;
  }
  pcInput = KlelSpace(psEval->pcRemainder);
  KlelFreeNode(&psEval);

  /* ExprList */
  psExprList = KlelExprList(pcInput, psContext);
  if (psExprList == NULL || psExprList->iType == KLEL_NODE_INVALID)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid guarded command", pcInput, NULL);
    KlelFreeNode(&psExprList);
    KlelFreeNode(&psExpr);
    return NULL;
  }

  psClose = KlelTerminal("^[)]", "close", KlelSpace(psExprList->pcRemainder));
  if (psClose == NULL || psClose->iType == KLEL_NODE_INVALID)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "empty or unterminated expression list", pcInput, NULL);
    KlelFreeNode(&psClose);
    KlelFreeNode(&psExprList);
    KlelFreeNode(&psExpr);
    return NULL;
  }

  /* Ensure that the first and second values in the expression list are strings. */
  if
  (
       psExprList->apsChildren[0] == NULL
    || psExprList->apsChildren[0]->apsChildren[0] == NULL
    || strcmp(psExprList->apsChildren[0]->acName, "string") != 0
    || strcmp(psExprList->apsChildren[0]->apsChildren[0]->acName, "fragment") != 0
    || psExprList->apsChildren[1] == NULL
    || psExprList->apsChildren[1]->apsChildren[0] == NULL
    || strcmp(psExprList->apsChildren[1]->acName, "string") != 0
    || strcmp(psExprList->apsChildren[1]->apsChildren[0]->acName, "fragment") != 0
  )
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "the first and second arguments to 'eval' must be constant strings", pcInput, NULL);
    KlelFreeNode(&psExprList);
    KlelFreeNode(&psExpr);
    return NULL;
  }

  /* Create the result. */
  psResult = KlelCreateNode("guard", pcInput, psClose->pcRemainder, 2, psExpr, psExprList);
  KLEL_ASSERT(psResult->apsChildren[KLEL_COMMAND_CHILD_INDEX] == psExprList);
  KlelFreeNode(&psClose);

  /* Check for success/failure codes. */
  pcInput = KlelSpace(psResult->pcRemainder);

  /* Success list. */
  psSigil = KlelTerminal("^(pass|fail)\\s*[[]", "sigil", pcInput);
  if (psSigil == NULL || psSigil->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psSigil);
    return psResult;
  }
  pcInput = KlelSpace(psSigil->pcRemainder);

  psCodes = KlelExprList(pcInput, psContext);
  if (psCodes == NULL || psCodes->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psResult);
    KlelFreeNode(&psSigil);
    return psCodes;
  }


  psClose = KlelTerminal("^[]]", "close", KlelSpace(psCodes->pcRemainder));
  if (psClose == NULL || psClose->iType == KLEL_NODE_INVALID)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "empty or unterminated expression list", pcInput, NULL);
    KlelFreeNode(&psResult);
    KlelFreeNode(&psSigil);
    KlelFreeNode(&psCodes);
    return NULL;
  }
  psResult->pcRemainder = psClose->pcRemainder;
  pcInput = KlelSpace(psResult->pcRemainder);
  KlelFreeNode(&psClose);

  if (psSigil != NULL && psCodes != NULL && psSigil->pcInput[0] == 'f')
  {
    psResult->apsChildren[KLEL_FAILURE_CHILD_INDEX] = psCodes;
  }
  else if (psSigil != NULL && psCodes != NULL && psSigil->pcInput[0] == 'p')
  {
    psResult->apsChildren[KLEL_SUCCESS_CHILD_INDEX] = psCodes;
  }

  KlelFreeNode(&psSigil);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * MulOperator = '*' | '/' | '%' | '&&' | '<<' | '>>'
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelMulOperator(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psResult = NULL;

  KLEL_ASSERT(pcInput != NULL);

  psResult = KlelTerminal("^([*]|/|%|[&][&]|>>|<<)", "op", KlelSpace(pcInput));
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    strncpy(psResult->acName, psResult->acString, KLEL_MAX_NAME);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * Number =  '0x' hexdigit {hexdigit}
 * Number =/ '0' {octdigit}
 * Number =/ digit {digit} ['.' digit {digit}] ['e' ['+' | '-'] digit {digit}]
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelNumber(const char *pcInput, KLEL_CONTEXT *psContext)
{
  char   *pcEnd    = NULL;
  KLEL_NODE *psResult = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume space. */
  pcInput = KlelSpace(pcInput);

  psResult = KlelTerminal("^[0-9]+[.][0-9]+(e[-+]?[0-9]+)?", "float", KlelSpace(pcInput));
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    errno = 0;
    psResult->dFloat = strtod(psResult->pcInput, &pcEnd);
    if (errno == ERANGE || pcEnd != psResult->pcRemainder)
    {
      KlelFreeNode(&psResult);
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid float literal", pcInput, NULL);
      return NULL;
    }
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^0o[0-7]+", "int", KlelSpace(pcInput));
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    errno = 0;
    psResult->llInteger = (int64_t)(strtoll(psResult->pcInput + 2, &pcEnd, 8));
    if (errno == ERANGE || pcEnd != psResult->pcRemainder)
    {
      KlelFreeNode(&psResult);
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid integer literal", pcInput, NULL);
      return NULL;
    }
    return psResult;
  }
  KlelFreeNode(&psResult);

  psResult = KlelTerminal("^(0x[A-Fa-f0-9]+|0*[1-9][0-9]*|0+)", "int", KlelSpace(pcInput));
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    errno = 0;
    psResult->llInteger = (int64_t)(strtoll(psResult->pcInput, &pcEnd, 0));
    if (errno == ERANGE || pcEnd != psResult->pcRemainder)
    {
      KlelFreeNode(&psResult);
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid integer literal", pcInput, NULL);
      return NULL;
    }

    return psResult;
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * Relation = '<' | '>' | '<=' | '>=' | '!=' | '==' | '=~' | '!~'
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelRelation(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psResult = NULL;

  KLEL_ASSERT(pcInput != NULL);

  psResult = KlelTerminal("^(<=|>=|>|<|!~|!=|=~|==)", "op", KlelSpace(pcInput));
  if (psResult != NULL && psResult->iType != KLEL_NODE_INVALID)
  {
    strncpy(psResult->acName, psResult->acString, KLEL_MAX_NAME);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * SimpleEpxression = Term {KlelAddOperator Term}
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelSimpleExpression(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psLeft  = NULL;
  KLEL_NODE *psOp    = NULL;
  KLEL_NODE *psRight = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Term */
  psLeft = KlelTerm(pcInput, psContext);
  if (psLeft == NULL || psLeft->iType == KLEL_NODE_INVALID)
  {
    return psLeft;
  }

  /* {KlelAddOperator Term} */
  psOp = KlelAddOperator(KlelSpace(psLeft->pcRemainder), psContext);
  if (psOp == NULL || psOp->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psOp);
    return psLeft;
  }

  while (psOp != NULL && psOp->iType != KLEL_NODE_INVALID)
  {
    psRight = KlelTerm(KlelSpace(psOp->pcRemainder), psContext);
    if (psRight == NULL || psRight->iType == KLEL_NODE_INVALID)
    {
      KlelFreeNode(&psOp);
      KlelFreeNode(&psLeft);

      return psRight;
    }

    psLeft = KlelCreateNode(psOp->acName, psRight->pcInput, psRight->pcRemainder, 2, psLeft, psRight);
    KlelFreeNode(&psOp);
    psOp   = KlelAddOperator(KlelSpace(psRight->pcRemainder), psContext);
  }

  if (psOp != NULL)
  {
    KLEL_ASSERT(psOp->iType == KLEL_NODE_INVALID);
    KlelFreeNode(&psOp);
  }

  return psLeft;
}


/*-
 ***********************************************************************
 *
 * String = '"' {character} '"'
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelString(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE  *psQuote     = NULL;
  KLEL_NODE  *psResult    = calloc(1, sizeof(KLEL_NODE));
  KLEL_NODE  *psSub       = NULL;
  size_t     szi          = 0;

  KLEL_ASSERT(pcInput != NULL);

  psQuote = KlelTerminal("^\"", "quote", KlelSpace(pcInput));
  if (psQuote == NULL || psQuote->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psResult);
    return psQuote;
  }
  pcInput = psQuote->pcRemainder;
  KlelFreeNode(&psQuote);

  if (psResult != NULL)
  {
    psResult->iType       = KLEL_NODE_NONTERMINAL;
    psResult->pcInput     = pcInput;
    psResult->pcRemainder = pcInput;
    psResult->szLength    = 0;
    strncpy(psResult->acName, "string", KLEL_MAX_NAME);

    psSub = KlelCharacter(pcInput, psContext);
    while (psSub != NULL && psSub->iType != KLEL_NODE_INVALID)
    {
      psResult->apsChildren[szi]  = psSub;
      psResult->pcRemainder       = psSub->pcRemainder;
      psResult->szLength         += psSub->szLength;

      psSub = KlelCharacter(psSub->pcRemainder, psContext);
      szi++;

      if (szi >= KLEL_MAX_CHILDREN)
      {
        KlelFreeNode(&psResult);
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "too many interpolations", pcInput, NULL);
        return NULL;
      }
    }

    if (psSub == NULL || psSub->iType == KLEL_NODE_INVALID)
    {
      KlelFreeNode(&psSub);
    }
  }

  psQuote = KlelTerminal("^\"", "quote", KlelSpace(psResult->pcRemainder));
  if (psQuote == NULL || psQuote->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psResult);
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unterminated string", pcInput, NULL);
    return psQuote;
  }
  psResult->pcRemainder = psQuote->pcRemainder;
  KlelFreeNode(&psQuote);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * Term = Factor {MulOperator Factor}
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelTerm(const char *pcInput, KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psLeft  = NULL;
  KLEL_NODE *psOp    = NULL;
  KLEL_NODE *psRight = NULL;

  KLEL_ASSERT(pcInput != NULL);

  /* Consume whitespace. */
  pcInput = KlelSpace(pcInput);

  /* Factor */
  psLeft = KlelFactor(pcInput, psContext);
  if (psLeft == NULL || psLeft->iType == KLEL_NODE_INVALID)
  {
    return psLeft;
  }

  /* {MulOperator Factor} */
  psOp = KlelMulOperator(KlelSpace(psLeft->pcRemainder), psContext);
  if (psOp == NULL || psOp->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psOp);

    return psLeft;
  }

  while (psOp != NULL && psOp->iType != KLEL_NODE_INVALID)
  {
    psRight = KlelFactor(KlelSpace(psOp->pcRemainder), psContext);
    if (psRight == NULL || psRight->iType == KLEL_NODE_INVALID)
    {
      KlelFreeNode(&psLeft);
      KlelFreeNode(&psOp);

      return psRight;
    }

    psLeft = KlelCreateNode(psOp->acName, psRight->pcInput, psRight->pcRemainder, 2, psLeft, psRight);
    KlelFreeNode(&psOp);
    psOp   = KlelMulOperator(KlelSpace(psRight->pcRemainder), psContext);
  }

  if (psOp != NULL)
  {
    KLEL_ASSERT(psOp->iType == KLEL_NODE_INVALID);
    KlelFreeNode(&psOp);
  }

  return psLeft;
}
