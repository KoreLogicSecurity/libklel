/*-
 ***********************************************************************
 *
 * $Id: inner-string.c,v 1.27 2012/11/20 16:38:37 rking Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#include "all-includes.h"

static KLEL_NODE_STRING_MAP gasKlelNodeStringMap[] =
{
  [KLEL_NODE_AND]             = {KlelStringOfBinaryOp,       "&"},
  [KLEL_NODE_AND_AND]         = {KlelStringOfBinaryOp,       "&&"},
  [KLEL_NODE_BANG]            = {KlelStringOfUnaryOp,        "!"},
  [KLEL_NODE_CALL]            = {KlelStringOfCall,           NULL},
  [KLEL_NODE_CARET]           = {KlelStringOfBinaryOp,       "^"},
  [KLEL_NODE_CONDITIONAL]     = {KlelStringOfConditional,    NULL},
  [KLEL_NODE_DESIGNATOR]      = {KlelStringOfDesignator,     NULL},
  [KLEL_NODE_DOT]             = {KlelStringOfBinaryOp,       "."},
  [KLEL_NODE_EQ_EQ]           = {KlelStringOfBinaryOp,       "=="},
  [KLEL_NODE_EXPRESSION]      = {KlelStringOfExpression,     NULL},
  [KLEL_NODE_FRAGMENT]        = {KlelStringOfFragment,       NULL},
  [KLEL_NODE_GT]              = {KlelStringOfBinaryOp,       ">"},
  [KLEL_NODE_GTE]             = {KlelStringOfBinaryOp,       ">="},
  [KLEL_NODE_GT_GT]           = {KlelStringOfBinaryOp,       ">>"},
  [KLEL_NODE_GUARDED_COMMAND] = {KlelStringOfGuardedCommand, NULL},
  [KLEL_NODE_INTEGER]         = {KlelStringOfInteger,        NULL},
  [KLEL_NODE_INTERP]          = {KlelStringOfInterpolation,  "{}"},
  [KLEL_NODE_LET]             = {KlelStringOfLet,            NULL},
  [KLEL_NODE_LIKE]            = {KlelStringOfBinaryOp,       "=~"},
  [KLEL_NODE_LT]              = {KlelStringOfBinaryOp,       "<"},
  [KLEL_NODE_LTE]             = {KlelStringOfBinaryOp,       "<="},
  [KLEL_NODE_LT_LT]           = {KlelStringOfBinaryOp,       "<<"},
  [KLEL_NODE_MINUS]           = {KlelStringOfBinaryOp,       "-"},
  [KLEL_NODE_NE]              = {KlelStringOfBinaryOp,       "!="},
  [KLEL_NODE_NEGATE]          = {KlelStringOfUnaryOp,        "-"},
  [KLEL_NODE_PERCENT]         = {KlelStringOfBinaryOp,       "%"},
  [KLEL_NODE_PIPE]            = {KlelStringOfBinaryOp,       "|"},
  [KLEL_NODE_PIPE_PIPE]       = {KlelStringOfBinaryOp,       "||"},
  [KLEL_NODE_PLUS]            = {KlelStringOfBinaryOp,       "+"},
  [KLEL_NODE_QUOTED_INTERP]   = {KlelStringOfInterpolation,  "()"},
  [KLEL_NODE_REAL]            = {KlelStringOfReal,           NULL},
  [KLEL_NODE_SLASH]           = {KlelStringOfBinaryOp,       "/"},
  [KLEL_NODE_STAR]            = {KlelStringOfBinaryOp,       "*"},
  [KLEL_NODE_TILDE]           = {KlelStringOfUnaryOp,        "~"},
  [KLEL_NODE_UNLIKE]          = {KlelStringOfBinaryOp,       "!~"},
};

/*-
 ***********************************************************************
 *
 * KlelStringNew
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringNew(void)
{
  /*-
   *********************************************************************
   *
   * This is allocated as a structure containing a pointer that also
   * points to allocated memory so that we can return the pointer to
   * allocated memory as a normal C string while freeing the container
   * structure.
   *
   *********************************************************************
   */

  KLEL_STRING *psString = calloc(1, sizeof(KLEL_STRING));
  if (psString != NULL)
  {
    psString->pcString = calloc(1, KLEL_INITIAL_STRING_SIZE);
    if (psString->pcString != NULL)
    {
      psString->szAlloc  = KLEL_INITIAL_STRING_SIZE;
      psString->szLength = 0;
    }
    else
    {
      free(psString);
      psString = NULL;
    }
  }

  return psString;
}


/*-
 ***********************************************************************
 *
 * KlelStringConcat
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringConcat(KLEL_STRING *psDest, KLEL_STRING *psSource)
{
  KLEL_STRING *psResult = NULL;

  KLEL_ASSERT(psDest    != NULL && psDest->pcString   != NULL);
  KLEL_ASSERT(psSource  != NULL && psSource->pcString != NULL);

  if (KlelStringReserve(psDest, psDest->szLength + psSource->szLength + 1))
  {
    memcpy(psDest->pcString + psDest->szLength, psSource->pcString, psSource->szLength);
    psDest->szLength += psSource->szLength;
    psDest->pcString[psDest->szLength] = 0;
    psResult = psDest;
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringConcatCString
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringConcatCString(KLEL_STRING *psDest, const char *pcSource)
{
  KLEL_STRING *psResult      = NULL;
  size_t      szSourceLength = 0;

  KLEL_ASSERT(psDest    != NULL && psDest->pcString   != NULL);
  KLEL_ASSERT(pcSource  != NULL);

  szSourceLength = strlen(pcSource);

  if (KlelStringReserve(psDest, psDest->szLength + szSourceLength + 1))
  {
    memcpy(psDest->pcString + psDest->szLength, pcSource, szSourceLength);
    psDest->szLength += szSourceLength;
    psDest->pcString[psDest->szLength] = 0;
    psResult = psDest;
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringReserve
 *
 ***********************************************************************
 */
int
KlelStringReserve(KLEL_STRING *psDest, size_t szLength)
{
  char *pcTemp = NULL;

  KLEL_ASSERT(psDest != NULL && psDest->pcString != NULL);

  if (szLength == 0)
  {
    szLength = KLEL_INITIAL_STRING_SIZE;
  }

  pcTemp = realloc(psDest->pcString, szLength);
  if (pcTemp != NULL)
  {
    psDest->pcString = pcTemp;
    psDest->szAlloc  = szLength;
  }

  return (psDest->szAlloc == szLength);
}


/*-
 ***********************************************************************
 *
 * KlelStringFree
 *
 ***********************************************************************
 */
void
KlelStringFree(KLEL_STRING *psString, int bFreeString)
{
  if (psString != NULL)
  {
    if (psString->pcString != NULL && bFreeString)
    {
      free(psString->pcString);
    }

    free(psString);
  }
}


/*-
 ***********************************************************************
 *
 * KlelStringPrintf
 *
 ***********************************************************************
 */
int
KlelStringPrintf(KLEL_STRING *psString, const char *pcFormat, ...)
{
  int iCount = 0;

  va_list vlArgs;

  va_start(vlArgs, pcFormat);
  iCount = vsnprintf(psString->pcString, psString->szAlloc, pcFormat, vlArgs);
  va_end(vlArgs);

  while (iCount >= (int)(psString->szAlloc))
  {
    va_start(vlArgs, pcFormat);
    iCount = vsnprintf(psString->pcString, psString->szAlloc - 1, pcFormat, vlArgs);
    va_end(vlArgs);

    if (!KlelStringReserve(psString, psString->szAlloc * 2))
    {
      return -1;
    }
  }

  psString->szLength += iCount;
  return 1;
}


/*-
 ***********************************************************************
 *
 * KlelInnerStringOfExpression
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelInnerStringOfExpression(KLEL_NODE *psNode, unsigned long ulFlags)
{
  KLEL_ASSERT(psNode        != NULL);
  KLEL_ASSERT(psNode->iType <= KLEL_NODE_UNLIKE && gasKlelNodeStringMap[psNode->iType].pfInstruction != NULL);

  return gasKlelNodeStringMap[psNode->iType].pfInstruction(psNode, gasKlelNodeStringMap[psNode->iType].pcString, ulFlags);
}


/*-
 ***********************************************************************
 *
 * KlelStringOfBinaryOp
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfBinaryOp(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();
  KLEL_STRING *psLeft   = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_OPERAND1_INDEX], ulFlags);
  KLEL_STRING *psRight  = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_OPERAND2_INDEX], ulFlags);

  if (psResult != NULL && psLeft != NULL && psRight != NULL)
  {
    switch (psNode->apsChildren[KLEL_OPERAND1_INDEX]->iType)
    {
      case KLEL_NODE_CALL:
      case KLEL_NODE_DESIGNATOR:
      case KLEL_NODE_FRAGMENT:
      case KLEL_NODE_INTEGER:
      case KLEL_NODE_INTERP:
      case KLEL_NODE_QUOTED_INTERP:
      case KLEL_NODE_REAL:
        KlelStringConcat(psResult, psLeft);
        break;

      default:
        KlelStringConcatCString(psResult, "(");
        KlelStringConcat(psResult, psLeft);
        KlelStringConcatCString(psResult, ")");
        break;
    }

    KlelStringConcatCString(psResult, " ");
    KlelStringConcatCString(psResult, pcString);
    KlelStringConcatCString(psResult, " ");

    switch (psNode->apsChildren[KLEL_OPERAND2_INDEX]->iType)
    {
      case KLEL_NODE_CALL:
      case KLEL_NODE_DESIGNATOR:
      case KLEL_NODE_FRAGMENT:
      case KLEL_NODE_INTEGER:
      case KLEL_NODE_INTERP:
      case KLEL_NODE_QUOTED_INTERP:
      case KLEL_NODE_REAL:
        KlelStringConcat(psResult, psRight);
        break;

      default:
        KlelStringConcat(psResult, psRight);
        break;
    }
  }

  KlelStringFree(psLeft,  1);
  KlelStringFree(psRight, 1);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfCall
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfCall(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult   = KlelStringNew();
  KLEL_STRING *psArgument = NULL;
  size_t      szi         = 0;

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "%s(", psNode->acFragment);

    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS && psNode->apsChildren[szi] != NULL; szi++)
    {
      if (psNode->apsChildren[szi] != NULL)
      {
        psArgument = KlelInnerStringOfExpression(psNode->apsChildren[szi], ulFlags);
        KlelStringConcat(psResult, psArgument);
        if (szi < KLEL_MAX_FUNC_ARGS -1 && psNode->apsChildren[szi + 1] != NULL)
        {
          KlelStringConcatCString(psResult, ", ");
        }
        KlelStringFree(psArgument, 1);
      }
    }

    KlelStringConcatCString(psResult, ")");
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfConditional
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfConditional(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult    = KlelStringNew();
  KLEL_STRING *psPredicate = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_PREDICATE_INDEX], ulFlags);
  KLEL_STRING *psIfTrue    = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_IFTRUE_INDEX],    ulFlags);
  KLEL_STRING *psIfFalse   = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_IFFALSE_INDEX],   ulFlags);

  if (psResult != NULL && psPredicate != NULL && psIfTrue != NULL && psIfFalse != NULL)
  {
    KlelStringConcatCString(psResult, "(");
    KlelStringConcatCString(psResult, "(");
    KlelStringConcat(psResult, psPredicate);
    KlelStringConcatCString(psResult, ") ? ");
    KlelStringConcat(psResult, psIfTrue);
    KlelStringConcatCString(psResult, " : ");
    KlelStringConcat(psResult, psIfFalse);
    KlelStringConcatCString(psResult, ")");
  }

  KlelStringFree(psPredicate, 1);
  KlelStringFree(psIfTrue, 1);
  KlelStringFree(psIfFalse, 1);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfDesignator
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfDesignator(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult    = KlelStringNew();

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "%s", psNode->acFragment);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfExpression
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfExpression(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psExpression = NULL;
  KLEL_STRING *psName       = NULL;
  KLEL_STRING *psResult     = KlelStringNew();

  if (psResult != NULL)
  {
    if (ulFlags & KLEL_EXPRESSION_PLUS_NAME && psNode->apsChildren[KLEL_LABEL_INDEX] != NULL)
    {
      psName = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_LABEL_INDEX], ulFlags);
      KlelStringConcat(psResult, psName);
      KlelStringConcatCString(psResult, " : ");
    }

    psExpression = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_EXPRESSION_INDEX], ulFlags);
    KlelStringConcat(psResult, psExpression);
  }

  KlelStringFree(psName, 1);
  KlelStringFree(psExpression, 1);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfFragment
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfFragment(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  if (psResult != NULL)
  {
    if (psNode->szLength == 1)
    {
      switch (psNode->acFragment[0])
      {
        case '%':
          KlelStringPrintf(psResult, "\"\\%%\"");
          break;

        case '\\':
          KlelStringPrintf(psResult, "\"\\\\\"");
          break;

        case '"':
          KlelStringPrintf(psResult, "\"\\\"\"");
          break;

        case '\r':
          KlelStringPrintf(psResult, "\"\\r\"");
          break;

        case '\n':
          KlelStringPrintf(psResult, "\"\\n\"");
          break;

        default:
          if (isprint(psNode->acFragment[0]))
          {
            KlelStringPrintf(psResult, "\"%c\"", psNode->acFragment[0]);
          }
          else
          {
            KlelStringPrintf(psResult, "\\x%02x", (int)(psNode->acFragment[0]));
          }
          break;
      }
    }
    else
    {
      KlelStringPrintf(psResult, "\"%s\"", psNode->acFragment);
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfGuardedCommand
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfGuardedCommand(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_NODE   *psCodeNode    = NULL;
  KLEL_NODE   *psGuard       = psNode->apsChildren[KLEL_EXPRESSION_INDEX];
  KLEL_STRING *psArgument    = NULL;
  KLEL_STRING *psName        = NULL;
  KLEL_STRING *psCode        = NULL;
  KLEL_STRING *psCommand     = KlelInnerStringOfExpression(psGuard->apsChildren[KLEL_COMMAND_INDEX],     ulFlags);
  KLEL_STRING *psInterpreter = KlelInnerStringOfExpression(psGuard->apsChildren[KLEL_INTERPRETER_INDEX], ulFlags);
  KLEL_STRING *psPredicate   = KlelInnerStringOfExpression(psGuard->apsChildren[KLEL_PREDICATE_INDEX],   ulFlags);
  KLEL_STRING *psResult      = KlelStringNew();
  size_t      szi            = 0;

  if (psResult != NULL && psPredicate != NULL)
  {
    if (ulFlags & KLEL_EXPRESSION_PLUS_NAME && psNode->apsChildren[KLEL_LABEL_INDEX] != NULL)
    {
      psName = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_LABEL_INDEX], ulFlags);
      KlelStringConcat(psResult, psName);
      KlelStringConcatCString(psResult, " : ");
      KlelStringFree(psName, 1);
    }

    if (ulFlags & KLEL_EXPRESSION_PLUS_COMMAND_LINE)
    {
      KlelStringConcatCString(psResult, "if (");
    }

    KlelStringConcat(psResult, psPredicate);

    if (ulFlags & KLEL_EXPRESSION_PLUS_COMMAND_LINE)
    {
      KlelStringConcatCString(psResult, ") then eval(");
      KlelStringConcat(psResult, psInterpreter);
      KlelStringConcatCString(psResult, ", ");
      KlelStringConcat(psResult, psCommand);

      if (psGuard->apsChildren[KLEL_ARGUMENT1_INDEX] != NULL)
      {
        KlelStringConcatCString(psResult, ", ");
        for (szi = 1; szi < KLEL_MAX_FUNC_ARGS && psGuard->apsChildren[szi] != NULL; szi++)
        {
          if (psGuard->apsChildren[szi] != NULL)
          {
            psArgument = KlelInnerStringOfExpression(psGuard->apsChildren[szi], ulFlags);
            KlelStringConcat(psResult, psArgument);
            if (szi < KLEL_MAX_FUNC_ARGS -1 && psGuard->apsChildren[szi + 1] != NULL)
            {
              KlelStringConcatCString(psResult, ", ");
            }
            KlelStringFree(psArgument, 1);
          }
        }
      }

      KlelStringConcatCString(psResult, ")");
    }
  }

  if (ulFlags & KLEL_EXPRESSION_PLUS_RETURN_CODES)
  {
    if (psGuard->apsChildren[KLEL_PASS_INDEX] != NULL || psGuard->apsChildren[KLEL_FAIL_INDEX] != NULL)
    {
      KlelStringConcatCString(psResult, psGuard->apsChildren[KLEL_PASS_INDEX] != NULL ? " pass [" : " fail [");
      psCodeNode = psGuard->apsChildren[psGuard->apsChildren[KLEL_PASS_INDEX] != NULL ? KLEL_PASS_INDEX : KLEL_FAIL_INDEX];
      for (; psCodeNode != NULL; psCodeNode = psCodeNode->apsChildren[0])
      {
        psCode = KlelInnerStringOfExpression(psCodeNode, ulFlags);
        KlelStringConcat(psResult, psCode);
        KlelStringFree(psCode, 1);
        if (psCodeNode->apsChildren[0] != NULL)
        {
          KlelStringConcatCString(psResult, ", ");
        }
      }
      KlelStringConcatCString(psResult, "]");
    }
  }

  KlelStringFree(psPredicate, 1);
  KlelStringFree(psCommand, 1);
  KlelStringFree(psInterpreter, 1);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfInteger
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfInteger(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "%" PRId64, psNode->llInteger);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfInterpolation
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfInterpolation(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "\"%%%c%s%c\"", pcString[0], psNode->acFragment, pcString[1]);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfLet
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfLet(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psDesignator = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_DESIGNATOR_INDEX], ulFlags);
  KLEL_STRING *psDefinition = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_DEFINITION_INDEX], ulFlags);
  KLEL_STRING *psExpression = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_EXPRESSION_INDEX], ulFlags);
  KLEL_STRING *psResult     = KlelStringNew();

  if (psResult != NULL && psDesignator != NULL && psDefinition != NULL && psExpression != NULL)
  {
    KlelStringConcatCString(psResult, "let ");
    KlelStringConcat(psResult, psDesignator);
    KlelStringConcatCString(psResult, " = ");
    KlelStringConcat(psResult, psDefinition);
    KlelStringConcatCString(psResult, " in ");
    KlelStringConcat(psResult, psExpression);
  }

  KlelStringFree(psDesignator, 1);
  KlelStringFree(psDefinition, 1);
  KlelStringFree(psExpression, 1);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfReal
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfReal(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "%g", psNode->dReal);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringOfUnaryOp
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringOfUnaryOp(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();
  KLEL_STRING *psLeft   = KlelInnerStringOfExpression(psNode->apsChildren[KLEL_OPERAND1_INDEX], ulFlags);

  if (psResult != NULL && psLeft)
  {
    KlelStringPrintf(psResult, " %s ", pcString);

    switch (psNode->apsChildren[KLEL_OPERAND1_INDEX]->iType)
    {
      case KLEL_NODE_CALL:
      case KLEL_NODE_DESIGNATOR:
      case KLEL_NODE_FRAGMENT:
      case KLEL_NODE_INTEGER:
      case KLEL_NODE_INTERP:
      case KLEL_NODE_QUOTED_INTERP:
      case KLEL_NODE_REAL:
        KlelStringConcat(psResult, psLeft);
        break;

      default:
        KlelStringConcatCString(psResult, "(");
        KlelStringConcat(psResult, psLeft);
        KlelStringConcatCString(psResult, ")");
        break;
    }
  }

  KlelStringFree(psLeft, 1);
  return psResult;
}
