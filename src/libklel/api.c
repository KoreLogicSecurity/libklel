/*-
 ***********************************************************************
 *
 * $Id: api.c,v 1.101 2012/11/20 20:11:23 rking Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#include "all-includes.h"

static char const *gpcContextAllocationError = "context allocation error";
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
  KLEL_CONTEXT *psContext  = calloc(1, sizeof(KLEL_CONTEXT));
  KLEL_NODE    *psCodeNode = NULL;
  size_t       szi         = 0;

  if (ulFlags & KLEL_MUST_SPECIFY_RETURN_CODES)
  {
    ulFlags |= KLEL_MUST_BE_GUARDED_COMMAND; /* The flag above implies this one. */
  }

  if (psContext != NULL)
  {
    psContext->pcInput         = pcInput;
    psContext->pfGetTypeOfVar  = pfGetTypeOfVar  != NULL ? pfGetTypeOfVar  : (KLEL_TYPE_CALLBACK)KlelGetTypeOfStdVar;
    psContext->pfGetValueOfVar = pfGetValueOfVar != NULL ? pfGetValueOfVar : (KLEL_VALUE_CALLBACK)KlelGetValueOfStdVar;
    psContext->pvData          = pvData;
    psContext->psExpression    = KlelRoot(psContext);
    psContext->aiCodes[0]      = 1; /* By default, exit code 0 is successful. */
    psContext->bIsValid        = 0;

    if (psContext->psExpression != NULL)
    {
      psContext->iExpressionType = KlelTypeCheck(psContext->psExpression, psContext);
      if (psContext->iExpressionType != KLEL_TYPE_UNKNOWN)
      {
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

        if (psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_PASS_INDEX] != NULL)
        {
          memset(psContext->aiCodes, 0, sizeof(psContext->aiCodes));
          psCodeNode = psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_PASS_INDEX];
          for (szi = 0; szi < 256 && psCodeNode != NULL; szi++)
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

        if (psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_FAIL_INDEX] != NULL)
        {
          memset(psContext->aiCodes, 1, sizeof(psContext->aiCodes));
          psCodeNode = psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_FAIL_INDEX];
          for (szi = 0; szi < 256 && psCodeNode != NULL; szi++)
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

        psContext->bIsValid = 1;
      }
    }
  }

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
  char *pcName = calloc(KLEL_MAX_NAME + 1, 1);

  KLEL_ASSERT(KlelIsValid(psContext));

  if (pcName != NULL)
  {
    if (KlelIsValid(psContext))
    {
      if (psContext->psExpression->apsChildren[KLEL_LABEL_INDEX] != NULL)
      {
        strncpy(pcName, psContext->psExpression->apsChildren[KLEL_LABEL_INDEX]->acFragment, KLEL_MAX_NAME);
      }
      else
      {
        snprintf(pcName, KLEL_MAX_NAME, "expr(%08" PRIx32 ")", KlelGetChecksum(psContext, KLEL_EXPRESSION_PLUS_EVERYTHING));
      }
    }
  }

  return pcName;
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
  size_t       szi         = 0;
  KLEL_COMMAND *psCommand  = calloc(1, sizeof(KLEL_COMMAND));
  KLEL_VALUE   *psArgument = NULL;
  char         *pcString   = NULL;
  size_t       szLength    = 0;

  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  if (psCommand != NULL && KlelIsGuardedCommand(psContext))
  {
    psArgument = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_INTERPRETER_INDEX], psContext);
    if (psArgument == NULL)
    {
      free(psCommand);
      return NULL;
    }
    pcString = KlelValueToString(psArgument, &szLength);
    if (pcString == NULL)
    {
      free(psCommand);
      KlelFreeResult(psArgument);
      return NULL;
    }
    strncpy(psCommand->pcInterpreter, pcString, KLEL_MAX_NAME);
    free(pcString);
    KlelFreeResult(psArgument);

    psArgument = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_COMMAND_INDEX], psContext);
    if (psArgument == NULL)
    {
      free(psCommand);
      return NULL;
    }
    pcString = KlelValueToString(psArgument, &szLength);
    if (pcString == NULL)
    {
      free(psCommand);
      KlelFreeResult(psArgument);
      return NULL;
    }
    strncpy(psCommand->pcProgram, pcString, KLEL_MAX_NAME);
    free(pcString);
    KlelFreeResult(psArgument);

    memcpy(psCommand->aiCodes, psContext->aiCodes, sizeof(psCommand->aiCodes));
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS && psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[szi] != NULL; szi++)
    {
      psArgument = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[szi], psContext);
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
      psCommand->ppcArgumentVector[szi] = pcString;
    }
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
  char       *pcResult = NULL;
  KLEL_VALUE *psValue  = NULL;
  size_t     szLength  = 0;

  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  if (KlelIsGuardedCommand(psContext))
  {
    psValue = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_INTERPRETER_INDEX], psContext);
    if (psValue != NULL)
    {
      pcResult = KlelValueToString(psValue, &szLength);
      KlelFreeResult(psValue);
    }
  }

  return pcResult;
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
  char       *pcResult = NULL;
  KLEL_VALUE *psValue  = NULL;
  size_t     szLength  = 0;

  KLEL_ASSERT(KlelIsGuardedCommand(psContext));

  if (KlelIsGuardedCommand(psContext))
  {
    psValue = KlelInnerExecute(psContext->psExpression->apsChildren[KLEL_EXPRESSION_INDEX]->apsChildren[KLEL_COMMAND_INDEX], psContext);
    if (psValue != NULL)
    {
      pcResult = KlelValueToString(psValue, &szLength);
      KlelFreeResult(psValue);
    }
  }

  return pcResult;
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
  size_t     szi          = 0;
  KLEL_ERROR *psError     = NULL;
  KLEL_ERROR *psNextError = NULL;

  if (psContext != NULL)
  {
    if (psContext->psExpression != NULL)
    {
      KlelFreeNode(psContext->psExpression);
    }

    if (psContext->psClosures != NULL)
    {
      for (szi = 0; szi < psContext->iClosureCount; szi++)
      {
        if (psContext->psClosures[szi].psValue != NULL)
        {
          KlelFreeResult(psContext->psClosures[szi].psValue);
        }
      }
      free(psContext->psClosures);
    }

    psError = psContext->psErrors;
    while (psError != NULL)
    {
      psNextError = psError->psNext;
      free(psError);
      psError = psNextError;
    }

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
  KLEL_VALUE *psTemp   = NULL;
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

        KLEL_ASSERT(pcTemp != NULL || szLength == 0);

        if (pcTemp == NULL && szLength != 0)
        {
          KlelFreeResult(psResult);
          va_end(vlArgs);
          return NULL;
        }

        psTemp = realloc(psResult, sizeof(KLEL_VALUE) + szLength + 1);
        if (psTemp == NULL)
        {
          KlelFreeResult(psResult);
          va_end(vlArgs);
          return NULL;
        }

        psResult           = psTemp;
        psResult->iType    = KLEL_EXPR_STRING;
        psResult->szLength = szLength;
        memcpy(psResult->acString, pcTemp, szLength);
        psResult->acString[szLength] = 0;
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

        strncpy(psResult->acString, pcTemp, KLEL_MAX_NAME);
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
KlelValueToQuotedString(const KLEL_VALUE *psValue, size_t *szLength)
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
  KLEL_ASSERT(szLength           != NULL);

  if (psValue == NULL || gpcKlelQuotedChars == NULL || gcKlelQuoteChar == 0 || szLength == NULL)
  {
    return NULL;
  }

  szQuotedChars = strlen(gpcKlelQuotedChars);

  pcString = KlelValueToString(psValue, szLength);
  if (pcString != NULL)
  {
    /* XXX - This is incredibly inefficient. */
    for (szi = 0; szi < *szLength; szi++)
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

    pcBuffer = calloc(*szLength + szCount + 1, sizeof(char));
    if (pcBuffer != NULL)
    {
      for (szi = 0; szi < *szLength; szi++)
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

      *szLength = *szLength + szCount;
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
KlelValueToString(const KLEL_VALUE *psValue, size_t *szLength)
{
  char *pcBuffer = calloc(1, KLEL_MAX_NAME + 1);

  KLEL_ASSERT(psValue  != NULL);
  KLEL_ASSERT(szLength != NULL);

  if (psValue != NULL && pcBuffer != NULL)
  {
    switch (psValue->iType)
    {
      case KLEL_EXPR_BOOLEAN:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%s", psValue->bBoolean ? "true" : "false");
        *szLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_REAL:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%g", psValue->dReal);
        *szLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_INTEGER:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%" PRId64, psValue->llInteger);
        *szLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_STRING:
        free(pcBuffer);
        pcBuffer = calloc(1, psValue->szLength + 1);
        if (pcBuffer != NULL)
        {
          memcpy(pcBuffer, psValue->acString, psValue->szLength);
          *szLength = psValue->szLength;
        }
        break;

      default:
        if (KLEL_IS_FUNCTION(psValue->iType))
        {
          pcBuffer = calloc(KLEL_MAX_NAME + 1, 1);
          snprintf(pcBuffer, KLEL_MAX_NAME, "\\%s", psValue->acString);
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
  char       *pcString = NULL;
  int        iLength   = 0;
  KLEL_ERROR *psError  = NULL;
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
    psError = (KLEL_ERROR *) calloc(1, (sizeof(KLEL_ERROR) + iLength));
    if (psError != NULL)
    {
      va_start(pVaList, pcFormat);
      vsnprintf(psError->acMessage, iLength, pcFormat, pVaList);
      va_end(pVaList);
      psError->psNext = psContext->psErrors;
      psContext->psErrors = psError;
    }
  }
}


/*-
 ***********************************************************************
 *
 * KlelGetFirstError
 *
 ***********************************************************************
 */
const char *
KlelGetFirstError(KLEL_CONTEXT *psContext)
{
  const char *pcResult = (psContext == NULL) ? gpcContextAllocationError : NULL;

  if (psContext != NULL && psContext->psErrors != NULL)
  {
    psContext->psNextError = psContext->psErrors->psNext;
    pcResult = psContext->psErrors->acMessage;
  }

  return pcResult;
}


/*-
 ***********************************************************************
 *
 * KlelGetNextError
 *
 ***********************************************************************
 */
const char *
KlelGetNextError(KLEL_CONTEXT *psContext)
{
  const char *pcResult = (psContext == NULL) ? gpcContextAllocationError : NULL;

  if (psContext != NULL && psContext->psNextError != NULL)
  {
    pcResult = psContext->psNextError->acMessage;
    psContext->psNextError = psContext->psNextError->psNext;
  }

  return pcResult;
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
 * KlelGetVersionCurrent
 *
 ***********************************************************************
 */
int
KlelGetVersionCurrent(void)
{
  return KLEL_VERSION_CURRENT;
}


/*-
 ***********************************************************************
 *
 * KlelGetVersionRevision
 *
 ***********************************************************************
 */
int
KlelGetVersionRevision(void)
{
  return KLEL_VERSION_REVISION;
}


/*-
 ***********************************************************************
 *
 * KlelGetVersionAge
 *
 ***********************************************************************
 */
int
KlelGetVersionAge(void)
{
  return KLEL_VERSION_AGE;
}
