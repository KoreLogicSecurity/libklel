/*-
 ***********************************************************************
 *
 * $Id: api.c,v 1.110 2019/07/31 15:59:27 klm Exp $
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

static char const *gpcContextAllocationError = "context allocation error";
static char const *gpcMemoryAllocationError = "memory allocation error";
static char        gcKlelQuoteChar        = KLEL_DEFAULT_QUOTE_CHAR;
static char const *gpcKlelQuotedChars     = KLEL_DEFAULT_QUOTED_CHARS;

/*-
 ***********************************************************************
 *
 * KlelCompile
 *
 ***********************************************************************
 */
KLEL_CONTEXT *
KlelCompile(const char *pcInput, unsigned long ulFlags, const KLEL_TYPE_CALLBACK pfGetTypeOfVar, const KLEL_VALUE_CALLBACK pfGetValueOfVar, void *pvData)
{
  KLEL_CONTEXT *psContext = NULL;
  KLEL_NODE    *psCodeNode = NULL;
  KLEL_STRING  *psString = NULL;
  KLEL_VALUE   *psValue = NULL;
  char         *pcName = NULL;
  char         *pcResult = NULL;
  char         *pcTemp = NULL;
  size_t        szIndex  = 0;
  size_t        szLength = 0;
  uint32_t      uiResult = 0;

  /*-
   *********************************************************************
   *
   * Conditionally set implied flags.
   *
   *********************************************************************
   */
  if (ulFlags & KLEL_MUST_SPECIFY_RETURN_CODES)
  {
    ulFlags |= KLEL_MUST_BE_GUARDED_COMMAND;
  }

  /*-
   *********************************************************************
   *
   * Allocate a new context structure and tag it as not yet valid.
   *
   *********************************************************************
   */
  psContext = calloc(1, sizeof(KLEL_CONTEXT));
  if (psContext == NULL)
  {
    return NULL;
  }
  psContext->bIsValid = 0;

  /*-
   *********************************************************************
   *
   * Initialize context members that were provided by the caller.
   *
   *********************************************************************
   */
  psContext->pcInput = pcInput;
  psContext->pfGetTypeOfVar = (pfGetTypeOfVar != NULL) ? pfGetTypeOfVar : (KLEL_TYPE_CALLBACK)KlelGetTypeOfStdVar;
  psContext->pfGetValueOfVar = (pfGetValueOfVar != NULL) ? pfGetValueOfVar : (KLEL_VALUE_CALLBACK)KlelGetValueOfStdVar;
  psContext->pvData = pvData;

  /*-
   *********************************************************************
   *
   * Parse the expression and verify that it returns a known type.
   *
   *********************************************************************
   */
  psContext->psExpression = KlelRoot(psContext);
  if (psContext->psExpression == NULL)
  {
    return psContext;
  }

  psContext->iExpressionType = KlelTypeCheck(psContext->psExpression, psContext);
  if (psContext->iExpressionType == KLEL_TYPE_UNKNOWN)
  {
    return psContext;
  }

  /*-
   *********************************************************************
   *
   * Initialize the expression's name. If the caller specified a name,
   * we simply copy it. Otherwise, we generate a name by calculating a
   * "standard" BSD checksum over the string representation of the
   * parsed expression.
   *
   *********************************************************************
   */
  pcName = calloc(KLEL_MAX_NAME + 1, 1);
  if (pcName == NULL)
  {
    KlelReportMemoryAllocationError(psContext);
    return psContext;
  }
  if (psContext->psExpression->apsChildren[KLEL_LABEL_INDEX] == NULL)
  {
    psString = KlelInnerStringOfExpression(psContext->psExpression, KLEL_EXPRESSION_PLUS_EVERYTHING);
    if (psString == NULL)
    {
      KlelReportMemoryAllocationError(psContext);
      return psContext;
    }
    szLength = strlen(psString->pcString);
    for (szIndex = 0; szIndex < szLength; szIndex++)
    {
      uiResult  = (uiResult >> 1) + ((uiResult & 1) << 15);
      uiResult += psString->pcString[szIndex];
      uiResult &= 0xFFFF;
    }
    KlelStringFree(psString, 1);
    snprintf(pcName, KLEL_MAX_NAME, "expr(%08" PRIx32 ")", uiResult);
  }
  else
  {
    pcTemp = SteelStringToCString(psContext->psExpression->apsChildren[KLEL_LABEL_INDEX]->psFragment);
    strncpy(pcName, pcTemp, KLEL_MAX_NAME);
    free(pcTemp);
  }
  psContext->pcName = pcName;

  /*-
   *********************************************************************
   *
   * Conditionally initialize the interpreter (guarded commands).
   *
   *********************************************************************
   */
  if (psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_INTERPRETER_INDEX] != NULL)
  {
    psValue = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_INTERPRETER_INDEX], psContext);
    if (psValue == NULL)
    {
      KlelReportMemoryAllocationError(psContext);
      return psContext;
    }
    pcResult = KlelValueToString(psValue, &szLength);
    if (pcResult == NULL)
    {
      KlelReportMemoryAllocationError(psContext);
      return psContext;
    }
    psContext->pcInterpreter = pcResult;
    KlelFreeResult(psValue);
  }


  /*-
   *********************************************************************
   *
   * Conditionally initialize the program (guarded commands).
   *
   *********************************************************************
   */
  if (psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_COMMAND_INDEX] != NULL)
  {
    psValue = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_COMMAND_INDEX], psContext);
    if (psValue == NULL)
    {
      KlelReportMemoryAllocationError(psContext);
      return psContext;
    }
    pcResult = KlelValueToString(psValue, &szLength);
    if (pcResult == NULL)
    {
      KlelReportMemoryAllocationError(psContext);
      return psContext;
    }
    psContext->pcProgram = pcResult;
    KlelFreeResult(psValue);
  }

  /*-
   *********************************************************************
   *
   * The default success exit code is zero (guarded commands). Thus,
   * all remaining exit codes (i.e., [1-255]) indicate a failure by
   * default.
   *
   *********************************************************************
   */
  psContext->aiCodes[0] = 1;

  /*-
   *********************************************************************
   *
   * Conditionally initialize success exit codes (guarded commands).
   *
   *********************************************************************
   */
  if (psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_PASS_INDEX] != NULL)
  {
    memset(psContext->aiCodes, 0, sizeof(psContext->aiCodes));
    psCodeNode = psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_PASS_INDEX];
    for (szIndex = 0; szIndex < 256 && psCodeNode != NULL; szIndex++)
    {
      KLEL_ASSERT(psCodeNode->iType == KLEL_NODE_INTEGER);
      if (psCodeNode->llInteger > 255)
      {
        KlelReportError(psContext, "return codes must be less than or equal to 255", NULL);
        return psContext;
      }
      psContext->aiCodes[psCodeNode->llInteger] = 1;
      psCodeNode = psCodeNode->apsChildren[0];
    }
  }

  /*-
   *********************************************************************
   *
   * Conditionally initialize failure exit codes (guarded commands).
   *
   *********************************************************************
   */
  if (psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_FAIL_INDEX] != NULL)
  {
    memset(psContext->aiCodes, 1, sizeof(psContext->aiCodes));
    psCodeNode = psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_FAIL_INDEX];
    for (szIndex = 0; szIndex < 256 && psCodeNode != NULL; szIndex++)
    {
      KLEL_ASSERT(psCodeNode->iType == KLEL_NODE_INTEGER);
      if (psCodeNode->llInteger > 255)
      {
        KlelReportError(psContext, "return codes must be less than or equal to 255", NULL);
        return psContext;
      }
      psContext->aiCodes[psCodeNode->llInteger] = 0;
      psCodeNode = psCodeNode->apsChildren[0];
    }
  }

  /*-
   *********************************************************************
   *
   * Do some sanity checks.
   *
   *********************************************************************
   */
  if (ulFlags & KLEL_MUST_BE_NAMED && psContext->psExpression->apsChildren[KLEL_LABEL_INDEX] == NULL)
  {
    KlelReportError(psContext, "expression must be named when KLEL_MUST_BE_NAMED is set", NULL);
    return psContext;
  }

  if ((ulFlags & KLEL_MUST_BE_GUARDED_COMMAND) && psContext->psExpression->iType != KLEL_NODE_GUARDED_COMMAND)
  {
    KlelReportError(psContext, "expression must be a guarded command when KLEL_MUST_BE_GUARDED_COMMAND is set", NULL);
    return psContext;
  }

  if
  (
       (ulFlags & KLEL_MUST_SPECIFY_RETURN_CODES)
    && psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_PASS_INDEX] == NULL
    && psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_FAIL_INDEX] == NULL
  )
  {
    KlelReportError(psContext, "expression must specify return codes when KLEL_MUST_SPECIFY_RETURN_CODES is set", NULL);
    return psContext;
  }

  /*-
   *********************************************************************
   *
   * Tag the context structure as valid.
   *
   *********************************************************************
   */
  psContext->bIsValid = 1;

  return psContext;
}


/*-
 ***********************************************************************
 *
 * KlelExecute
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelExecute(KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psValue = NULL;

  KLEL_ASSERT(psContext != NULL);
  KLEL_ASSERT(KlelIsValid(psContext));

  if (KlelIsValid(psContext))
  {
    KlelClearError(psContext);
    psValue = KlelInnerExecute(psContext->psExpression, psContext);
  }

  return psValue;
}


/*-
 ***********************************************************************
 *
 * KlelIsValid
 *
 ***********************************************************************
 */
int
KlelIsValid(const KLEL_CONTEXT *psContext)
{
  return psContext != NULL && psContext->bIsValid && psContext->psExpression != NULL;
}


/*-
 ***********************************************************************
 *
 * KlelGetChecksum
 *
 ***********************************************************************
 */
uint32_t
KlelGetChecksum(const KLEL_CONTEXT *psContext, unsigned long ulFlags)
{
  char     *pcExpression = NULL;
  size_t   szLength      = 0;
  size_t   szi           = 0;
  uint32_t uiResult      = 0;

  KLEL_ASSERT(psContext != NULL);
  KLEL_ASSERT(KlelIsValid(psContext));

  pcExpression = KlelExpressionToString(psContext, ulFlags);
  if (pcExpression != NULL)
  {
    /*-
     ***********************************************************************
     *
     * This is the "standard" original BSD checksum algorithm.
     *
     ***********************************************************************
     */

    szLength = strlen(pcExpression);

    for (szi = 0; szi < szLength; szi++)
    {
      uiResult  = (uiResult >> 1) + ((uiResult & 1) << 15);
      uiResult += pcExpression[szi];
      uiResult &= 0xFFFF;
    }

    free(pcExpression);
  }

  return uiResult;
}


/*-
 ***********************************************************************
 *
 * KlelGetName
 *
 ***********************************************************************
 */
char *
KlelGetName(const KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(KlelIsValid(psContext));

  return (KlelIsValid(psContext)) ? psContext->pcName : NULL;
}


/*-
 ***********************************************************************
 *
 * KlelGetPrivateData
 *
 ***********************************************************************
 */
void *
KlelGetPrivateData(const KLEL_CONTEXT *psContext)
{
  void *pvResult = NULL;

  KLEL_ASSERT(psContext != NULL);

  if (psContext != NULL)
  {
    pvResult = psContext->pvData;
  }

  return pvResult;
}


/*-
 ***********************************************************************
 *
 * KlelSetPrivateData
 *
 ***********************************************************************
 */
void
KlelSetPrivateData(KLEL_CONTEXT *psContext, void *pvData)
{
  KLEL_ASSERT(psContext != NULL);

  if (psContext != NULL)
  {
    psContext->pvData = pvData;
  }
}


/*-
 ***********************************************************************
 *
 * KlelGetTypeOfExpression
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelGetTypeOfExpression(const KLEL_CONTEXT *psContext)
{
  KLEL_EXPR_TYPE iType = KLEL_TYPE_UNKNOWN;

  KLEL_ASSERT(KlelIsValid(psContext));

  if (KlelIsValid(psContext))
  {
    iType = psContext->iExpressionType;
  }

  return iType;
}


/*-
 ***********************************************************************
 *
 * KlelIsGuardedCommand
 *
 ***********************************************************************
 */
int
KlelIsGuardedCommand(const KLEL_CONTEXT *psContext)
{
  int bIsGuardedCommand = 0;

  KLEL_ASSERT(KlelIsValid(psContext));

  if (KlelIsValid(psContext))
  {
    bIsGuardedCommand = psContext->psExpression->iType == KLEL_NODE_GUARDED_COMMAND;
  }

  return bIsGuardedCommand;
}


/*-
 ***********************************************************************
 *
 * KlelGetCommand
 *
 ***********************************************************************
 */
KLEL_COMMAND *
KlelGetCommand(KLEL_CONTEXT *psContext)
{
  KLEL_COMMAND *psCommand  = calloc(1, sizeof(KLEL_COMMAND));
  KLEL_VALUE   *psArgument = NULL;
  char         *pcString   = NULL;
  size_t       szIndex     = 0;
  size_t       szLength    = 0;

  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  if (psCommand != NULL && KlelIsGuardedCommand(psContext))
  {
    strncpy(psCommand->acInterpreter, psContext->pcInterpreter, KLEL_MAX_NAME);
    strncpy(psCommand->acProgram, psContext->pcProgram, KLEL_MAX_NAME);
    for (szIndex = 0; szIndex < KLEL_MAX_FUNC_ARGS && psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[szIndex] != NULL; szIndex++)
    {
      psArgument = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[szIndex], psContext);
      if (psArgument == NULL)
      {
        free(psCommand);
        return NULL;
      }
      pcString = KlelValueToString(psArgument, &szLength);
      KlelFreeResult(psArgument);
      if (pcString == NULL)
      {
        free(psCommand);
        return NULL;
      }
      psCommand->szArgumentCount++;
      psCommand->ppcArgumentVector[szIndex] = pcString;
    }
    memcpy(psCommand->aiCodes, psContext->aiCodes, sizeof(psCommand->aiCodes));
  }

  return psCommand;
}


/*-
 ***********************************************************************
 *
 * KlelGetCommandInterpreter
 *
 ***********************************************************************
 */
char *
KlelGetCommandInterpreter(KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  return (KlelIsGuardedCommand(psContext)) ? psContext->pcInterpreter : NULL;
}


/*-
 ***********************************************************************
 *
 * KlelGetCommandProgram
 *
 ***********************************************************************
 */
char *
KlelGetCommandProgram(KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  return (KlelIsGuardedCommand(psContext)) ? psContext->pcProgram : NULL;
}


/*-
 ***********************************************************************
 *
 * KlelIsSuccessReturnCode
 *
 ***********************************************************************
 */
int
KlelIsSuccessReturnCode(const KLEL_CONTEXT *psContext, unsigned int uiCode)
{
  int iResult = 0;

  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  if (KlelIsGuardedCommand(psContext) && uiCode < 256)
  {
    iResult = psContext->aiCodes[uiCode];
  }

  return iResult;
}


/*-
 ***********************************************************************
 *
 * KlelExpressionToString
 *
 ***********************************************************************
 */
char *
KlelExpressionToString(const KLEL_CONTEXT *psContext, unsigned long ulFlags)
{
  char        *pcResult = NULL;
  KLEL_STRING *psString = NULL;

  KLEL_ASSERT(KlelIsValid(psContext));

  if (KlelIsValid(psContext))
  {
    psString = KlelInnerStringOfExpression(psContext->psExpression, ulFlags);
    if (psString != NULL)
    {
      pcResult = psString->pcString;
      KlelStringFree(psString, 0);
    }
  }

  return pcResult;
}


/*-
 ***********************************************************************
 *
 * KlelFreeContext
 *
 ***********************************************************************
 */
void
KlelFreeContext(KLEL_CONTEXT *psContext)
{
  size_t      szIndex = 0;

  if (psContext != NULL)
  {
    if (psContext->pcName != NULL)
    {
      free(psContext->pcName);
    }
    if (psContext->pcInterpreter != NULL)
    {
      free(psContext->pcInterpreter);
    }
    if (psContext->pcProgram != NULL)
    {
      free(psContext->pcProgram);
    }
    if (psContext->psExpression != NULL)
    {
      KlelFreeNode(psContext->psExpression);
    }
    if (psContext->psClosures != NULL)
    {
      for (szIndex = 0; szIndex < psContext->iClosureCount; szIndex++)
      {
        if (psContext->psClosures[szIndex].psValue != NULL)
        {
          KlelFreeResult(psContext->psClosures[szIndex].psValue);
        }
      }
      free(psContext->psClosures);
    }
    KlelClearError(psContext);
    free(psContext);
  }
}


/*-
 ***********************************************************************
 *
 * KlelFreeCommand
 *
 ***********************************************************************
 */
void
KlelFreeCommand(KLEL_COMMAND *psCommand)
{
  size_t szi = 0;

  if (psCommand != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS; szi++)
    {
      if (psCommand->ppcArgumentVector[szi] != NULL)
      {
        free(psCommand->ppcArgumentVector[szi]);
      }
    }

    free(psCommand);
  }
}

/*-
 ***********************************************************************
 *
 * KlelClearError
 *
 ***********************************************************************
 */
void
KlelClearError(KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(KlelIsValid(psContext));

  if (KlelIsValid(psContext))
  {
    if
    (
         psContext->pcError != NULL
      && psContext->pcError != gpcContextAllocationError
      && psContext->pcError != gpcMemoryAllocationError
    )
    {
      free(psContext->pcError);
    }
    psContext->pcError = NULL;
  }
}


/*-
 ***********************************************************************
 *
 * KlelCreateValue
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelCreateValue(KLEL_EXPR_TYPE iType, ...)
{
  va_list    vlArgs;
  size_t     szLength  = 0;
  KLEL_VALUE *psResult = calloc(1, sizeof(KLEL_VALUE) + KLEL_MAX_NAME + 1);
  const char *pcTemp   = NULL;

  if (psResult != NULL)
  {
    psResult->iType = iType;
    va_start(vlArgs, iType);

    switch (iType)
    {
      case KLEL_TYPE_BOOLEAN:
        psResult->bBoolean = va_arg(vlArgs, unsigned int);
        break;

      case KLEL_TYPE_REAL:
        psResult->dReal = va_arg(vlArgs, double);
        break;

      case KLEL_EXPR_INTEGER:
        psResult->llInteger = va_arg(vlArgs, int64_t);
        break;

      case KLEL_EXPR_STRING:
        szLength = va_arg(vlArgs, size_t);
        pcTemp   = va_arg(vlArgs, const char *);
        
        if (pcTemp == NULL)
        {
          KlelFreeResult(psResult);
          va_end(vlArgs);
          return NULL;
        }

        psResult->psString = SteelCreateFragment(szLength, pcTemp);
        if (psResult->psString == NULL && szLength != 0)
        {
          KlelFreeResult(psResult);
          va_end(vlArgs);
          return NULL;
        }
        psResult->szLength = szLength;
        break;

      default:
        pcTemp = va_arg(vlArgs, const char *);
        psResult->fFunction = va_arg(vlArgs, KLEL_VALUE *(*)(KLEL_VALUE **, void *));

        KLEL_ASSERT(KLEL_IS_FUNCTION(iType));
        KLEL_ASSERT(pcTemp              != NULL);
        KLEL_ASSERT(psResult->fFunction != NULL);
        if (pcTemp == NULL || psResult->fFunction == NULL || !KLEL_IS_FUNCTION(iType))
        {
          KlelFreeResult(psResult);
          va_end(vlArgs);
          return NULL;
        }
        
        psResult->psString = SteelCreateFragment(strlen(pcTemp), pcTemp);
        break;
    }
  }

  va_end(vlArgs);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelSetQuotedChars
 *
 ***********************************************************************
 */
void
KlelSetQuotedChars(const char *pcChars)
{
  KLEL_ASSERT(pcChars != NULL);

  if (pcChars != NULL)
  {
    gpcKlelQuotedChars = pcChars;
  }
}


/*-
 ***********************************************************************
 *
 * KlelSetQuoteChar
 *
 ***********************************************************************
 */
void
KlelSetQuoteChar(char cChar)
{
  gcKlelQuoteChar = cChar;
}


/*-
 ***********************************************************************
 *
 * KlelValueToQuotedString
 *
 ***********************************************************************
 */
char *
KlelValueToQuotedString(const KLEL_VALUE *psValue, size_t *pszLength)
{
  size_t  szi           = 0;
  size_t  szj           = 0;
  size_t  szk           = 0;
  size_t  szCount       = 0;
  size_t  szQuotedChars = 0;
  char    *pcBuffer     = NULL;
  char    *pcString     = NULL;

  KLEL_ASSERT(psValue            != NULL);
  KLEL_ASSERT(gpcKlelQuotedChars != NULL);
  KLEL_ASSERT(gcKlelQuoteChar    != 0);
  KLEL_ASSERT(pszLength           != NULL);

  if (psValue == NULL || gpcKlelQuotedChars == NULL || gcKlelQuoteChar == 0 || pszLength == NULL)
  {
    return NULL;
  }

  szQuotedChars = strlen(gpcKlelQuotedChars);

  pcString = KlelValueToString(psValue, pszLength);
  if (pcString != NULL)
  {
    /* XXX - This is incredibly inefficient. */
    for (szi = 0; szi < *pszLength; szi++)
    {
      for (szj = 0; szj < szQuotedChars; szj++)
      {
        if (pcString[szi] == gpcKlelQuotedChars[szj])
        {
          szCount++;
        }
      }
    }

    if (szCount == 0)
    {
      return pcString;
    }

    pcBuffer = calloc(*pszLength + szCount + 1, sizeof(char));
    if (pcBuffer != NULL)
    {
      for (szi = 0; szi < *pszLength; szi++)
      {
        for (szj = 0; szj < szQuotedChars; szj++)
        {
          if (pcString[szi] == gpcKlelQuotedChars[szj])
          {
            pcBuffer[szk] = gcKlelQuoteChar;
            szk++;
          }
        }

        pcBuffer[szk] = pcString[szi];
        szk++;
      }

      *pszLength = *pszLength + szCount;
    }

    free(pcString);
  }

  return pcBuffer;
}


/*-
 ***********************************************************************
 *
 * KlelValueToString
 *
 ***********************************************************************
 */
char *
KlelValueToString(const KLEL_VALUE *psValue, size_t *pszLength)
{
  KLEL_STRING_NODE *psStringNode = NULL;
  char *pcBuffer = calloc(1, KLEL_MAX_NAME + 1);
  int iOff = 0;

  KLEL_ASSERT(psValue  != NULL);
  KLEL_ASSERT(pszLength != NULL);

  if (psValue != NULL && pcBuffer != NULL)
  {
    switch (psValue->iType)
    {
      case KLEL_EXPR_BOOLEAN:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%s", psValue->bBoolean ? "true" : "false");
        *pszLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_REAL:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%g", psValue->dReal);
        *pszLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_INTEGER:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%" PRId64, psValue->llInteger);
        *pszLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_STRING:
        free(pcBuffer);
        if (psValue->psString != NULL)
        {
          pcBuffer = SteelStringToCString(psValue->psString);
        }
        else if (psValue->szLength == 0)
        {
          pcBuffer = calloc(1, 1);
          if (pcBuffer == NULL)
          {
            return NULL;
          }
        }
        else
        {
          return NULL;
        }
        *pszLength = psValue->szLength;
        break;

      default:
        if (KLEL_IS_FUNCTION(psValue->iType))
        {
          free(pcBuffer);
          KLEL_ASSERT(psValue->psString != NULL);
          pcBuffer = SteelStringToCString(psValue->psString);
        }
        else
        {
          KLEL_ASSERT(0); /* Data structure corruption. */
          return NULL;
        }
    }
  }

  return pcBuffer;
}


/*-
 ***********************************************************************
 *
 * KlelFreeNode
 *
 ***********************************************************************
 */
void
KlelFreeNode(KLEL_NODE *psResult)
{
  size_t szi = 0;

  if (psResult != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_CHILDREN; szi++)
    {
      if (psResult->apsChildren[szi] != NULL)
      {
        KlelFreeNode(psResult->apsChildren[szi]);
      }
    }
    SteelFreeString(psResult->psFragment);
    free(psResult);
  }
}


/*-
 ***********************************************************************
 *
 * KlelFreeResult
 *
 ***********************************************************************
 */
void
KlelFreeResult(KLEL_VALUE *psResult)
{
  if (psResult != NULL)
  {
    SteelFreeString(psResult->psString);
    free(psResult);
  }
}


/*-
 ***********************************************************************
 *
 * KlelReportError
 *
 ***********************************************************************
 */
void
KlelReportError(KLEL_CONTEXT *psContext, const char *pcFormat, ...)
{
  char       *pcError  = NULL;
  char       *pcString = NULL;
  int        iLength   = 0;
  va_list    pVaList;

  KLEL_ASSERT(psContext != NULL);
  KLEL_ASSERT(pcFormat  != NULL);

  if (psContext != NULL)
  {
    va_start(pVaList, pcFormat);
    for (pcString = (char *) pcFormat, iLength = 0; pcString != NULL; pcString = va_arg(pVaList, char *))
    {
      iLength += strlen(pcString);
    }
    iLength++; /* Add one for the terminating NULL byte. */
    va_end(pVaList);
    pcError = (char *) realloc(psContext->pcError, iLength + 1);
    if (pcError != NULL)
    {
      va_start(pVaList, pcFormat);
      vsnprintf(pcError, iLength, pcFormat, pVaList);
      va_end(pVaList);
      psContext->pcError = pcError;
    }
  }
}


/*-
 ***********************************************************************
 *
 * KlelReportMemoryAllocationError
 *
 ***********************************************************************
 */
void
KlelReportMemoryAllocationError(KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psContext != NULL);

  if (psContext != NULL)
  {
    KlelClearError(psContext);
    psContext->pcError = (char *)gpcMemoryAllocationError; /* FIXME - This is violating the type system in all sorts of ways. */
  }
}


/*-
 ***********************************************************************
 *
 * KlelGetError
 *
 ***********************************************************************
 */
const char *
KlelGetError(KLEL_CONTEXT *psContext)
{
  return (psContext == NULL) ? gpcContextAllocationError : psContext->pcError;
}


/*-
 ***********************************************************************
 *
 * KlelGetFirstError (Note that this is legacy function.)
 *
 ***********************************************************************
 */
const char *
KlelGetFirstError(KLEL_CONTEXT *psContext)
{
  return KlelGetError(psContext);
}


/*-
 ***********************************************************************
 *
 * KlelGetNextError (Note that this is legacy function.)
 *
 ***********************************************************************
 */
const char *
KlelGetNextError(KLEL_CONTEXT *psContext)
{
  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelInnerGetTypeOfVar
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelInnerGetTypeOfVar(KLEL_CONTEXT *psContext, const char *pcName, void *pvData)
{
  KLEL_EXPR_TYPE iResult = KLEL_TYPE_UNKNOWN;

  KLEL_ASSERT(psContext != NULL);
  KLEL_ASSERT(pcName    != NULL);
  KLEL_ASSERT(pcName[0] != 0);

  if (psContext != NULL && pcName != NULL && pcName[0] != 0)
  {
    if (psContext->pfGetTypeOfVar != NULL)
    {
      iResult = psContext->pfGetTypeOfVar(pcName, (void *)psContext);

      if (iResult == KLEL_TYPE_UNKNOWN && psContext->pfGetTypeOfVar != (KLEL_TYPE_CALLBACK)KlelGetTypeOfStdVar)
      {
        iResult = KlelGetTypeOfStdVar(pcName, (void *)psContext);
      }
    }
    else
    {
      iResult = KlelGetTypeOfStdVar(pcName, (void *)psContext);
    }
  }

  return iResult;
}


/*-
 ***********************************************************************
 *
 * KlelInnerGetValueOfVar
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelInnerGetValueOfVar(KLEL_CONTEXT *psContext, const char *pcName, void *pvData)
{
  KLEL_VALUE *psResult = NULL;

  KLEL_ASSERT(KlelIsValid(psContext));
  KLEL_ASSERT(pcName    != NULL);
  KLEL_ASSERT(pcName[0] != 0);

  if (KlelIsValid(psContext) && pcName != NULL && pcName[0] != 0)
  {
    if (psContext->pfGetValueOfVar != NULL)
    {
      psResult = psContext->pfGetValueOfVar(pcName, (void *)psContext);
    }

    if (psResult == NULL || psResult->iType == KLEL_TYPE_UNKNOWN)
    {
      KlelFreeResult(psResult);
      if (psContext->pfGetTypeOfVar != NULL && psContext->pfGetTypeOfVar != (KLEL_TYPE_CALLBACK)KlelGetTypeOfStdVar)
      {
        if (psContext->pfGetTypeOfVar(pcName, (void *)psContext) == KLEL_TYPE_UNKNOWN)
        {
          psResult = KlelGetValueOfStdVar(pcName, (void *)psContext);
        }
      }
      else
      {
        psResult = KlelGetValueOfStdVar(pcName, (void *)psContext);
      }
    }
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelIsConstantString
 *
 ***********************************************************************
 */
int
KlelIsConstantString(KLEL_NODE *psNode)
{
  KLEL_ASSERT(psNode != NULL);

  if (psNode != NULL)
  {
    if (psNode->iType == KLEL_NODE_FRAGMENT)
    {
      return 1;
    }

    if (psNode->iType == KLEL_NODE_DOT)
    {
      return KlelIsConstantString(psNode->apsChildren[KLEL_OPERAND1_INDEX]) && KlelIsConstantString(psNode->apsChildren[KLEL_OPERAND2_INDEX]);
    }
  }

  return 0;
}


/*-
 ***********************************************************************
 *
 * KlelConstantStringLength
 *
 ***********************************************************************
 */
size_t
KlelConstantStringLength(KLEL_NODE *psNode)
{
  KLEL_ASSERT(psNode != NULL);
  KLEL_ASSERT(KlelIsConstantString(psNode));

  if (psNode != NULL)
  {
    if (psNode->iType == KLEL_NODE_FRAGMENT)
    {
      return psNode->szLength;
    }

    if (psNode->iType == KLEL_NODE_DOT)
    {
      return KlelConstantStringLength(psNode->apsChildren[KLEL_OPERAND1_INDEX]) + KlelConstantStringLength(psNode->apsChildren[KLEL_OPERAND2_INDEX]);
    }
  }

  return 0;
}


/*-
 ***********************************************************************
 *
 * KlelGetReleaseMajor
 *
 ***********************************************************************
 */
int
KlelGetReleaseMajor(void)
{
  return KLEL_RELEASE_MAJOR;
}


/*-
 ***********************************************************************
 *
 * KlelGetReleaseMinor
 *
 ***********************************************************************
 */
int
KlelGetReleaseMinor(void)
{
  return KLEL_RELEASE_MINOR;
}


/*-
 ***********************************************************************
 *
 * KlelGetReleaseNumber
 *
 ***********************************************************************
 */
uint32_t
KlelGetReleaseNumber(void)
{
  return KLEL_RELEASE_NUMBER;
}


/*-
 ***********************************************************************
 *
 * KlelGetReleasePatch
 *
 ***********************************************************************
 */
int
KlelGetReleasePatch(void)
{
  return KLEL_RELEASE_PATCH;
}


/*-
 ***********************************************************************
 *
 * KlelGetReleaseString
 *
 ***********************************************************************
 */
const char *
KlelGetReleaseString(void)
{
  return PACKAGE_VERSION;
}


/*-
 ***********************************************************************
 *
 * KlelGetVersionCurrent (Note that this is legacy function.)
 *
 ***********************************************************************
 */
int
KlelGetVersionCurrent(void)
{
  return KLEL_LIBRARY_CURRENT;
}


/*-
 ***********************************************************************
 *
 * KlelGetVersionRevision (Note that this is legacy function.)
 *
 ***********************************************************************
 */
int
KlelGetVersionRevision(void)
{
  return KLEL_LIBRARY_REVISION;
}


/*-
 ***********************************************************************
 *
 * KlelGetVersionAge (Note that this is legacy function.)
 *
 ***********************************************************************
 */
int
KlelGetVersionAge(void)
{
  return KLEL_LIBRARY_AGE;
}


/*-
 ***********************************************************************
 *
 * KlelGetLibraryCurrent
 *
 ***********************************************************************
 */
int
KlelGetLibraryCurrent(void)
{
  return KLEL_LIBRARY_CURRENT;
}


/*-
 ***********************************************************************
 *
 * KlelGetLibraryRevision
 *
 ***********************************************************************
 */
int
KlelGetLibraryRevision(void)
{
  return KLEL_LIBRARY_REVISION;
}


/*-
 ***********************************************************************
 *
 * KlelGetLibraryAge
 *
 ***********************************************************************
 */
int
KlelGetLibraryAge(void)
{
  return KLEL_LIBRARY_AGE;
}


/*-
 ***********************************************************************
 *
 * KlelGetReleaseString
 *
 ***********************************************************************
 */
const char *
KlelGetLibraryVersion(void)
{
  return KLEL_LIBRARY_VERSION;
}
