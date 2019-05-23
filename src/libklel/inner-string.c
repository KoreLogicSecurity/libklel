/*-
 ***********************************************************************
 *
 * $Id: inner-string.c,v 1.17 2012/04/26 00:53:31 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#define _BSD_SOURCE /* For v?snprintf. */
#include "all-includes.h"

static KLEL_NODE_STRING_MAP gasKlelNodeStringMap[] =
{
  {"args",      KlelArgsToString},
  {"call",      KlelCallToString},
  {"cond",      KlelConditionalToString},
  {"float",     KlelFloatToString},
  {"guard",     KlelGuardToString},
  {"int",       KlelIntegerToString},
  {"^",         KlelOpToString},
  {"<<",        KlelOpToString},
  {"<=",        KlelOpToString},
  {"<",         KlelOpToString},
  {"=~",        KlelOpToString},
  {"==",        KlelOpToString},
  {">=",        KlelOpToString},
  {">>",        KlelOpToString},
  {">",         KlelOpToString},
  {"||",        KlelOpToString},
  {"|",         KlelOpToString},
  {"-",         KlelOpToString},
  {"!~",        KlelOpToString},
  {"!=",        KlelOpToString},
  {"/",         KlelOpToString},
  {".",         KlelOpToString},
  {"*",         KlelOpToString},
  {"&",         KlelOpToString},
  {"&&",        KlelOpToString},
  {"%",         KlelOpToString},
  {"+",         KlelOpToString},
  {"root",      KlelRootToString},
  {"string",    KlelStringToString},
  {"bnot",      KlelUnopToString, "~"},
  {"neg",       KlelUnopToString, "-"},
  {"not",       KlelUnopToString, "!"},
  {"var",       KlelVarToString}
};


/*-
 ***********************************************************************
 *
 * KlelStringNew()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringNew(void)
{
  /*-
   *********************************************************************
   *
   * This is allocated with as a structure containing a pointer that
   * also points to allocated memory so that we can return the pointer
   * to allocated memory as a normal C string while freeing the
   * container structure.
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
 * KlelStringConcat()
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
 * KlelStringConcatCString()
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
 * KlelStringReserve()
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
 * KlelStringFree()
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
 * KlelStringPrintf()
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
 * KlelInnerExpressionToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelInnerExpressionToString(KLEL_NODE *psNode, unsigned long ulFlags)
{
  size_t szi = 0;

  KLEL_ASSERT(psNode != NULL);

  for (szi = 0; szi < sizeof(gasKlelNodeStringMap) / sizeof(KLEL_NODE_STRING_MAP); szi++)
  {
    if (strcmp(gasKlelNodeStringMap[szi].pcName, psNode->acName) == 0)
    {
      return gasKlelNodeStringMap[szi].pfInstruction(psNode, gasKlelNodeStringMap[szi].pcString, ulFlags);
    }
  }

  KLEL_ASSERT(0); /* Data structure corruption. */
  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelCallToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelCallToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();
  KLEL_STRING *psArgs   = NULL;

  KLEL_ASSERT(psNode != NULL && strcmp(psNode->apsChildren[1]->acName, "args") == 0);

  if (psResult != NULL)
  {
    psArgs = KlelArgsToString(psNode->apsChildren[1], NULL, ulFlags);
    if (psArgs != NULL)
    {
      KlelStringPrintf(psResult, "%s(", psNode->apsChildren[0]->acString);
      KlelStringConcat(psResult, psArgs);
      KlelStringConcatCString(psResult, ")");
      KlelStringFree(psArgs, 1);
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelConditionalToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelConditionalToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult     = KlelStringNew();
  KLEL_STRING *psExpression = NULL;
  KLEL_STRING *psTrue       = NULL;
  KLEL_STRING *psFalse      = NULL;

  KLEL_ASSERT(psNode != NULL && strcmp(psNode->acName, "cond") == 0);

  if (psResult != NULL)
  {
    psExpression = KlelInnerExpressionToString(psNode->apsChildren[0], ulFlags);
    if (psExpression != NULL)
    {
      psTrue = KlelInnerExpressionToString(psNode->apsChildren[0], ulFlags);
      if (psTrue != NULL)
      {
        psFalse = KlelInnerExpressionToString(psNode->apsChildren[1], ulFlags);
        if (psFalse != NULL)
        {
          KlelStringPrintf(psResult, "(%s) ? (%s) : (%s)", psExpression, psTrue, psFalse);
          KlelStringFree(psFalse, 1);
        }
        KlelStringFree(psTrue, 1);
      }
      KlelStringFree(psExpression, 1);
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelFloatToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelFloatToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  KLEL_ASSERT(psNode != NULL);

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "%g", psNode->dFloat);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelGuardToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelGuardToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult  = KlelStringNew();
  KLEL_STRING *psGuard   = NULL;
  KLEL_STRING *psCommand = NULL;
  KLEL_STRING *psCodes   = NULL;

  KLEL_ASSERT(psNode != NULL && strcmp(psNode->acName, "guard") == 0);

  if (psResult != NULL)
  {
    psGuard = KlelInnerExpressionToString(psNode->apsChildren[0], ulFlags);
    if (psGuard != NULL)
    {
      KlelStringPrintf(psResult, "if (%s) then ", psGuard->pcString);
      KlelStringFree(psGuard, 1);

      if (KLEL_EXPRESSION_PLUS_COMMAND_LINE & ulFlags)
      {
        KlelStringConcatCString(psResult, " eval(");

        psCommand = KlelInnerExpressionToString(psNode->apsChildren[KLEL_COMMAND_CHILD_INDEX], 0);
        if (psCommand != NULL)
        {
          KlelStringConcat(psResult, psCommand);
          KlelStringFree(psCommand, 1);
        }

        KlelStringConcatCString(psResult, ")");
      }
    }

    if (KLEL_EXPRESSION_PLUS_RETURN_CODES & ulFlags)
    {
      if (psNode->apsChildren[KLEL_SUCCESS_CHILD_INDEX] != NULL)
      {
        psCodes = KlelInnerExpressionToString(psNode->apsChildren[KLEL_SUCCESS_CHILD_INDEX], ulFlags);
        if (psCodes != NULL)
        {
          KlelStringConcatCString(psResult, " pass [");
          KlelStringConcat(psResult, psCodes);
          KlelStringConcatCString(psResult, "]");
          KlelStringFree(psCodes, 1);
        }
      }
      else if (psNode->apsChildren[KLEL_FAILURE_CHILD_INDEX] != NULL)
      {
        psCodes = KlelInnerExpressionToString(psNode->apsChildren[KLEL_FAILURE_CHILD_INDEX], ulFlags);
        if (psCodes != NULL)
        {
          KlelStringConcatCString(psResult, " fail [");
          KlelStringConcat(psResult, psCodes);
          KlelStringConcatCString(psResult, "]");
          KlelStringFree(psCodes, 1);
        }
      }
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelIntegerToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelIntegerToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  KLEL_ASSERT(psNode != NULL);

  if (psResult != NULL)
  {
    KlelStringPrintf(psResult, "%" PRId64, psNode->llInteger);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelOpToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelOpToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();
  KLEL_STRING *psLeft   = KlelInnerExpressionToString(psNode->apsChildren[0], ulFlags);
  KLEL_STRING *psRight  = KlelInnerExpressionToString(psNode->apsChildren[1], ulFlags);

  KLEL_ASSERT(psNode != NULL);

  if (psResult != NULL && psLeft != NULL && psRight != NULL)
  {
    if (psNode->apsChildren[0]->iType == KLEL_NODE_NONTERMINAL)
    {
      KlelStringConcatCString(psResult, "(");
      KlelStringConcat(psResult, psLeft);
      KlelStringConcatCString(psResult, ")");
    }
    else
    {
      KlelStringConcat(psResult, psLeft);
    }

    KlelStringConcatCString(psResult, " ");
    KlelStringConcatCString(psResult, psNode->acName);
    KlelStringConcatCString(psResult, " ");

    if (psNode->apsChildren[1]->iType == KLEL_NODE_NONTERMINAL)
    {
      KlelStringConcatCString(psResult, "(");
      KlelStringConcat(psResult, psRight);
      KlelStringConcatCString(psResult, ")");
    }
    else
    {
      KlelStringConcat(psResult, psRight);
    }
  }

  KlelStringFree(psLeft, 1);
  KlelStringFree(psRight, 1);

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelRootToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelRootToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult     = KlelStringNew();
  KLEL_STRING *psExpression = NULL;
  char        *psName       = NULL;

  KLEL_ASSERT(psNode != NULL && strcmp(psNode->acName, "root") == 0);

  if (psResult != NULL)
  {
    if (KLEL_EXPRESSION_PLUS_NAME & ulFlags)
    {
      psName = KlelGetName(psNode);
      if (psName != NULL)
      {
        KlelStringPrintf(psResult, "%s : ", psName);
        free(psName);
      }
    }

    psExpression = KlelInnerExpressionToString(psNode->apsChildren[KLEL_EXPRESSION_CHILD_INDEX], ulFlags);
    if (psExpression != NULL)
    {
      KlelStringConcat(psResult, psExpression);
      KlelStringFree(psExpression, 1);
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelStringToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelStringToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  char        acBuf[8]  = {0};
  size_t      szi       = 0;
  KLEL_STRING *psResult = KlelStringNew();

  KLEL_ASSERT(psNode != NULL && strcmp(psNode->acName, "string") == 0);

  if (psResult != NULL)
  {
    KlelStringConcatCString(psResult, "\"");
    for (szi = 0; szi < KLEL_MAX_CHILDREN && psNode->apsChildren[szi] != NULL; szi++)
    {
      if (strcmp(psNode->apsChildren[szi]->acName, "fragment") == 0)
      {
        KlelStringConcatCString(psResult, psNode->apsChildren[szi]->acString);
      }
      else if (strcmp(psNode->apsChildren[szi]->acName, "char") == 0)
      {
        snprintf(acBuf, sizeof(acBuf) - 1, "\\x%02x", psNode->apsChildren[szi]->cChar);
        KlelStringConcatCString(psResult, acBuf);
      }
      else if (strcmp(psNode->apsChildren[szi]->acName, "quote") == 0)
      {
        KlelStringConcatCString(psResult, "%(");
        KlelStringConcatCString(psResult, psNode->apsChildren[szi]->acString);
        KlelStringConcatCString(psResult, ")");
      }
      else if (strcmp(psNode->apsChildren[szi]->acName, "var") == 0)
      {
        KlelStringConcatCString(psResult, "%{");
        KlelStringConcatCString(psResult, psNode->apsChildren[szi]->acString);
        KlelStringConcatCString(psResult, "}");
      }
      else
      {
        KLEL_ASSERT(0); /* Data structure corruption. */
      }
    }
    KlelStringConcatCString(psResult, "\"");
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelUnopToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelUnopToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult  = KlelStringNew();
  KLEL_STRING *psOperand = NULL;

  KLEL_ASSERT(psNode != NULL);

  if (psResult != NULL)
  {
    KlelStringConcatCString(psResult, pcString);

    if (psNode->apsChildren[0]->iType == KLEL_NODE_NONTERMINAL)
    {
      KlelStringConcatCString(psResult, "(");
    }

    psOperand = KlelInnerExpressionToString(psNode->apsChildren[0], ulFlags);
    if (psOperand != NULL)
    {
      KlelStringConcat(psResult, psOperand);
      KlelStringFree(psOperand, 1);
    }

    if (psNode->apsChildren[0]->iType == KLEL_NODE_NONTERMINAL)
    {
      KlelStringConcatCString(psResult, ")");
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelVarToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelVarToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  KLEL_STRING *psResult = KlelStringNew();

  KLEL_ASSERT(psNode != NULL);

  if (psResult != NULL && strcmp(psNode->acName, "var") == 0)
  {
    KlelStringConcatCString(psResult, psNode->acString);
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelArgsToString()
 *
 ***********************************************************************
 */
KLEL_STRING *
KlelArgsToString(KLEL_NODE *psNode, const char *pcString, unsigned long ulFlags)
{
  size_t      szi         = 0;
  KLEL_STRING *psResult   = KlelStringNew();
  KLEL_STRING *psArgument = NULL;

  KLEL_ASSERT(psNode != NULL);

  if (psResult != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_CHILDREN && psNode->apsChildren[szi] != NULL; szi++)
    {
      psArgument = KlelInnerExpressionToString(psNode->apsChildren[szi], ulFlags);
      if (psArgument != NULL)
      {
        KlelStringConcat(psResult, psArgument);
        KlelStringFree(psArgument, 1);
      }

      if (szi < KLEL_MAX_CHILDREN - 1)
      {
        if (psNode->apsChildren[szi + 1] != NULL)
        {
          KlelStringConcatCString(psResult, ", ");
        }
      }
    }
  }

  return psResult;
}
