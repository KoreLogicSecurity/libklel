/*-
 ***********************************************************************
 *
 * $Id: api.c,v 1.76 2012/04/23 16:40:22 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#define _BSD_SOURCE /* For v?snprintf */
#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * Character quoting values.
 *
 ***********************************************************************
 */
char *gpcKlelQuotedChars = KLEL_QUOTED_CHARS;
char  gcKlelQuoteChar    = KLEL_QUOTE_CHAR;

/*-
 ***********************************************************************
 *
 * Callback definitions.
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE *(*KlelGetFuncDesc)(const char *pcName, KLEL_CONTEXT *psContext) = KlelGetStdLibFuncDesc;
KLEL_EXPR_TYPE (*KlelGetTypeOfVar)(const char *pcName, KLEL_CONTEXT *psContext) = KlelGetTypeOfStdVar;
KLEL_VALUE *(*KlelGetValueOfVar)(const char *pcName, KLEL_CONTEXT *psContext)   = KlelGetValueOfStdVar;
void (*KlelReportError)(KLEL_CONTEXT *psContext, const char *pcFormat, ...)     = KlelDefaultReportError;

/*-
 ***********************************************************************
 *
 * KlelClearErrors
 *
 ***********************************************************************
 */
void
KlelClearErrors(KLEL_CONTEXT *psContext)
{
  KLEL_ERROR *psError = NULL;
  KLEL_ERROR *psNext  = NULL;

  KLEL_ASSERT(psContext != NULL);

  if (psContext != NULL)
  {
    psError = psContext->psErrorList;
    while (psError != NULL)
    {
      psNext = psError->psNext;
      free(psError);
      psError = psNext;
    }

    psContext->psErrorList = NULL;
    psContext->psNextError = NULL;
  }
}


/*-
 ***********************************************************************
 *
 * KlelCompile
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCompile(const char *pcInput, unsigned long ulFlags, KLEL_CONTEXT *psContext)
{
  int       bSuccess  = 0;
  KLEL_NODE *psResult = NULL;

  KLEL_ASSERT(pcInput != NULL);
  KLEL_ASSERT(psContext != NULL);

  KlelClearErrors(psContext);

  if (pcInput == NULL || strlen(pcInput) == 0)
  {
    KlelReportError(psContext, "%s", "input is empty", NULL);
    return NULL;
  }

  psResult = KlelParse(pcInput, psContext);

  if (psResult == NULL || psResult->iType == KLEL_NODE_INVALID)
  {
    KlelFreeNode(&psResult);
    return NULL; /* The errors on the error stack will explain what happened. */
  }

  if (KlelTypeCheck(psResult, psContext) == KLEL_EXPR_UNKNOWN)
  {
    KlelFreeNode(&psResult);
    return NULL; /* The errors on the error stack will explain what happened. */
  }

  bSuccess = 1; /* Compile succeeded. */

  if (ulFlags & KLEL_MUST_SPECIFY_RETURN_CODES || ulFlags & KLEL_MUST_SPECIFY_COMMAND_ARGS)
  {
    ulFlags |= KLEL_MUST_BE_GUARDED_COMMAND; /* The two flags above imply this. */
  }

  if (KlelIsGuardedCommand(psResult))
  {
    if
    (
         ulFlags & KLEL_MUST_SPECIFY_RETURN_CODES
      && psResult->apsChildren[KLEL_EXPRESSION_CHILD_INDEX]->apsChildren[KLEL_SUCCESS_CHILD_INDEX] == NULL
      && psResult->apsChildren[KLEL_EXPRESSION_CHILD_INDEX]->apsChildren[KLEL_FAILURE_CHILD_INDEX] == NULL
    )
    {
      KlelReportError(psContext, "%s", "expression must specify one or more return codes when KLEL_MUST_SPECIFY_RETURN_CODES is set", NULL);
      bSuccess = 0;
    }

    if
    (
         ulFlags & KLEL_MUST_SPECIFY_COMMAND_ARGS
      && strcmp(psResult->apsChildren[KLEL_EXPRESSION_CHILD_INDEX]->apsChildren[KLEL_COMMAND_CHILD_INDEX]->acName, "args") != 0
    )
    {
      KlelReportError(psContext, "%s", "expression must specify command as an array of arguments when KLEL_MUST_SPECIFY_COMMAND_ARGS is set", NULL);
      bSuccess = 0;
    }
  }
  else /* Is not a guarded command. */
  {
    if (ulFlags & KLEL_MUST_SPECIFY_RETURN_CODES)
    {
      KlelReportError(psContext, "%s", "expression must specify one or more return codes when KLEL_MUST_SPECIFY_RETURN_CODES is set", NULL);
    }

    if (ulFlags & KLEL_MUST_SPECIFY_COMMAND_ARGS)
    {
      KlelReportError(psContext, "%s", "expression must specify command as an array of arguments when KLEL_MUST_SPECIFY_COMMAND_ARGS is set", NULL);
    }
  }

  if (ulFlags & KLEL_MUST_BE_GUARDED_COMMAND && !KlelIsGuardedCommand(psResult))
  {
    KlelReportError(psContext, "%s", "expression must be a guarded command when KLEL_MUST_BE_GUARDED_COMMAND is set", NULL);
    bSuccess = 0;
  }

  if (ulFlags & KLEL_MUST_BE_NAMED && psResult->apsChildren[KLEL_NAME_CHILD_INDEX] == NULL)
  {
    KlelReportError(psContext, "%s", "expression must be named when KLEL_MUST_BE_NAMED is set", NULL);
    bSuccess = 0;
  }

  if (bSuccess == 0)
  {
    KlelFreeNode(&psResult);
    psResult = NULL; /* The errors on the stack will let the user know what went wrong. */
  }

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelCreateContext
 *
 ***********************************************************************
 */
KLEL_CONTEXT *
KlelCreateContext(void *pvData)
{
  KLEL_CONTEXT *psContext = calloc(1, sizeof(KLEL_CONTEXT));

  if (psContext != NULL)
  {
    psContext->pvData = pvData;
  }

  return psContext;
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
      case KLEL_EXPR_BOOLEAN:
        psResult->bBoolean = va_arg(vlArgs, unsigned int);
        break;

      case KLEL_EXPR_FLOAT:
        psResult->dFloat = va_arg(vlArgs, double);
        break;

      case KLEL_EXPR_FUNCTION_BOOLEAN:
      case KLEL_EXPR_FUNCTION_FLOAT:
      case KLEL_EXPR_FUNCTION_INTEGER:
      case KLEL_EXPR_FUNCTION_STRING:
        pcTemp = va_arg(vlArgs, const char *);
        psResult->fFunction = va_arg(vlArgs, KLEL_VALUE *(*)(KLEL_VALUE **, void *));

        KLEL_ASSERT(pcTemp              != NULL);
        KLEL_ASSERT(psResult->fFunction != NULL);
        if (pcTemp == NULL || psResult->fFunction == NULL)
        {
          KlelFreeResult(psResult);
          va_end(vlArgs);
          return NULL;
        }

        strncpy(psResult->acString, pcTemp, KLEL_MAX_NAME);
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
        /* Invalid type specification. */
        KLEL_ASSERT(0);
        KlelFreeResult(psResult);
        va_end(vlArgs);
        return NULL;
    }
  }

  va_end(vlArgs);
  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelDefaultReportError
 *
 ***********************************************************************
 */
void
KlelDefaultReportError(KLEL_CONTEXT *psContext, const char *pcFormat, ...)
{
  char       *pcString = NULL;
  int        iLength   = 0;
  KLEL_ERROR *psError  = NULL;
  va_list    pVaList;

  KLEL_ASSERT(pcFormat != NULL);

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
      vsnprintf(psError->acError, iLength, pcFormat, pVaList);
      va_end(pVaList);
      psError->psNext = psContext->psErrorList;
      psContext->psErrorList = psError;
    }
  }
}


/*-
 ***********************************************************************
 *
 * KlelExecute
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelExecute(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot                           != NULL);
  KLEL_ASSERT(psRoot->iType                    != KLEL_NODE_INVALID);
  KLEL_ASSERT(KlelTypeCheck(psRoot, psContext) != KLEL_EXPR_UNKNOWN);

  KlelClearErrors(psContext);

  if (psRoot == NULL)
  {
    KlelReportError(psContext, "%s", "input is null", NULL);
    return NULL;
  }

  if (psRoot->iType == KLEL_NODE_INVALID)
  {
    KlelReportError(psContext, "%s", "input is invalid", NULL);
    return NULL;
  }

  if (KlelTypeCheck(psRoot, psContext) == KLEL_EXPR_UNKNOWN)
  {
    KlelReportError(psContext, "%s", "expression is ill-typed", NULL);
    return NULL;
  }

  return KlelInnerExecute(psRoot, psContext);
}


/*-
 ***********************************************************************
 *
 * KlelExpressionToString
 *
 ***********************************************************************
 */
char *
KlelExpressionToString(KLEL_NODE *psNode, unsigned long ulFlags)
{
  KLEL_STRING *psString = KlelInnerExpressionToString(psNode, ulFlags);
  char        *pcResult = (psString == NULL) ? NULL : psString->pcString;

  KlelStringFree(psString, 0); /* Free the KLEL_STRING structure, but not its internal char * buffer which we return. */

  return pcResult;
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
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS && psCommand->ppcArgumentVector[szi] != NULL; szi++)
    {
      free(psCommand->ppcArgumentVector[szi]);
    }
    if (psCommand->pcInterpreter != NULL)
    {
      free(psCommand->pcInterpreter);
    }
    if (psCommand->pcProgram != NULL)
    {
      free(psCommand->pcProgram);
    }
    free(psCommand);
  }
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
  KLEL_ERROR *psError = NULL;
  KLEL_ERROR *psNext  = NULL;

  if (psContext != NULL)
  {
    psError = psContext->psErrorList;
    while (psError != NULL)
    {
      psNext = psError->psNext;
      free(psError);
      psError = psNext;
    }
    free(psContext);
  }
}


/*-
 ***********************************************************************
 *
 * KlelFreeNode
 *
 ***********************************************************************
 */
void
KlelFreeNode(KLEL_NODE **psNode)
{
  size_t szi = 0;

  if (psNode != NULL && *psNode != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_CHILDREN; szi++)
    {
      KlelFreeNode(&((*psNode)->apsChildren[szi]));
    }

    free(*psNode);
    *psNode = NULL;
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
 * KlelGetChecksum
 *
 ***********************************************************************
 */
uint32_t
KlelGetChecksum(KLEL_NODE *psNode, unsigned long ulFlags)
{
  size_t szIndex     = 0;
  uint32_t uiSum     = 0;
  char *pcExpression = KlelExpressionToString(psNode, ulFlags);

  if (pcExpression != NULL)
  {
    for (szIndex = 0; szIndex < strlen(pcExpression); szIndex++)
    {
      uiSum += pcExpression[szIndex];
    }
    uiSum &= 0x00ffffff;
    uiSum |= ((uint32_t)strlen(pcExpression) << 24);

    free(pcExpression);
  }

  return uiSum;
}


/*-
 ***********************************************************************
 *
 * KlelGetCommand
 *
 ***********************************************************************
 */
KLEL_COMMAND *
KlelGetCommand(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext)
{
  char         *pcNew    = NULL;
  KLEL_COMMAND *psResult = calloc(1, sizeof(KLEL_COMMAND));
  KLEL_VALUE   *psValue  = NULL;
  size_t       szLength  = 0;
  size_t       szOffset  = 0;
  size_t       szi       = 0;

  KLEL_ASSERT(psExpression != NULL);
  KLEL_ASSERT(psResult     != NULL);

  /*-
   *********************************************************************
   *
   * Ensure we have an expression, a RHS (the second child), and a
   * result. If we don't the user is using the library incorrectly.
   *
   *********************************************************************
   */
  if (psExpression == NULL)
  {
    KlelReportError(psContext, "%s", "input is null", NULL);
    KlelFreeCommand(psResult);
    return NULL;
  }

  if (psResult == NULL)
  {
    KlelReportError(psContext, "%s", "out of memory", NULL);
    KlelFreeCommand(psResult);
    return NULL;
  }

  if (psExpression == NULL || psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX] == NULL)
  {
    KlelReportError(psContext, "%s", "input is null", NULL);
    KlelFreeCommand(psResult);
    return NULL;
  }

  if (strcmp(psExpression->acName, "root") == 0)
  {
    KLEL_ASSERT(psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] != NULL);
    psExpression = psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX];
  }

  if (psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX] == NULL || strcmp(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->acName, "args") != 0)
  {
    KlelReportError(psContext, "%s", "input is not a guarded command", NULL);
    KlelFreeCommand(psResult);
    return NULL;
  }

  psValue = KlelExecute(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[0], psContext);
  if (psValue == NULL)
  {
    return NULL; /* The execution error is placed in the context by KlelExecute. */
  }
  psResult->pcInterpreter = KlelValueToString(psValue, &szLength);
  KlelFreeResult(psValue);

  psValue = KlelExecute(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[1], psContext);
  if (psValue == NULL)
  {
    return NULL; /* The execution error is placed in the context by KlelExecute. */
  }
  psResult->pcProgram = KlelValueToString(psValue, &szLength);
  KlelFreeResult(psValue);

  for (szi = 1; szi < KLEL_MAX_FUNC_ARGS && psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[szi] != NULL; szi++)
  {
    psValue = KlelExecute(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[szi], psContext);
    if (psValue == NULL)
    {
      KlelFreeCommand(psResult);
      return NULL; /* The execution error is placed in the context by KlelExecute. */
    }
    psResult->ppcArgumentVector[szi - 1] = KlelValueToString(psValue, &szLength);
    KlelFreeResult(psValue);
  }
  psResult->szArgumentCount = szi - 1;

  return psResult;
}


/*-
 ***********************************************************************
 *
 * KlelGetCommandInterpreter
 *
 ***********************************************************************
 */
char *
KlelGetCommandInterpreter(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psValue  = NULL;
  char       *pcResult = NULL;
  size_t     szLength  = 0;

  if (psExpression == NULL || psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX] == NULL)
  {
    KlelReportError(psContext, "%s", "input is null", NULL);
    return NULL;
  }

  if (strcmp(psExpression->acName, "root") == 0)
  {
    KLEL_ASSERT(psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] != NULL);
    psExpression = psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX];
  }

  if (psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX] == NULL || strcmp(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->acName, "args") != 0)
  {
    KlelReportError(psContext, "%s", "input is not a guarded command", NULL);
    return NULL;
  }

  psValue = KlelExecute(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[0], psContext);
  if (psValue == NULL)
  {
    return NULL; /* The execution error is placed in the context by KlelExecute. */
  }
  pcResult = KlelValueToString(psValue, &szLength);
  KlelFreeResult(psValue);

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
KlelGetCommandProgram(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext)
{
  KLEL_VALUE *psValue  = NULL;
  char       *pcResult = NULL;
  size_t     szLength  = 0;

  if (psExpression == NULL || psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX] == NULL)
  {
    KlelReportError(psContext, "%s", "input is null", NULL);
    return NULL;
  }

  if (strcmp(psExpression->acName, "root") == 0)
  {
    KLEL_ASSERT(psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] != NULL);
    psExpression = psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX];
  }

  if (psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX] == NULL || strcmp(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->acName, "args") != 0)
  {
    KlelReportError(psContext, "%s", "input is not a guarded command", NULL);
    return NULL;
  }

  psValue = KlelExecute(psExpression->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[1], psContext);
  if (psValue == NULL)
  {
    return NULL; /* The execution error is placed in the context by KlelExecute. */
  }
  pcResult = KlelValueToString(psValue, &szLength);
  KlelFreeResult(psValue);

  return pcResult;
}


/*-
 ***********************************************************************
 *
 * KlelGetFirstError
 *
 ***********************************************************************
 */
char *
KlelGetFirstError(KLEL_CONTEXT *psContext)
{
  KLEL_ERROR *psError = NULL;

  KLEL_ASSERT(psContext != NULL);

  if (psContext != NULL)
  {
    psError = psContext->psNextError = psContext->psErrorList;
    if (psError != NULL)
    {
      psContext->psNextError = psError->psNext;
      return psError->acError;
    }
  }

  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelGetName
 *
 ***********************************************************************
 */
char *
KlelGetName(KLEL_NODE *psNode)
{
  char *pcBuf = calloc(1, KLEL_MAX_NAME + 1);

  if (pcBuf != NULL && psNode != NULL)
  {
    if (psNode->apsChildren[KLEL_NAME_CHILD_INDEX] != NULL)
    {
      strncpy(pcBuf, psNode->apsChildren[KLEL_NAME_CHILD_INDEX]->acString, KLEL_MAX_NAME);
    }
    else
    {
      snprintf(pcBuf, KLEL_MAX_NAME, "expr(%" PRIx32 ")", KlelGetChecksum(psNode, KLEL_EXPRESSION_ONLY));
    }
  }

  return pcBuf;
}


/*-
 ***********************************************************************
 *
 * KlelGetNextError
 *
 ***********************************************************************
 */
char *
KlelGetNextError(KLEL_CONTEXT *psContext)
{
  KLEL_ERROR *psError = NULL;

  if (psContext != NULL)
  {
    psError = psContext->psNextError;
    if (psError != NULL)
    {
      psContext->psNextError = psError->psNext;
      return psError->acError;
    }
  }

  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelGetQuoteChar
 *
 ***********************************************************************
 */
char
KlelGetQuoteChar(void)
{
  KLEL_ASSERT(!isalnum(gcKlelQuoteChar) && !isspace(gcKlelQuoteChar) && isprint(gcKlelQuoteChar));

  return gcKlelQuoteChar;
}


/*-
 ***********************************************************************
 *
 * KlelGetQuotedChars
 *
 ***********************************************************************
 */
const char *
KlelGetQuotedChars(void)
{
  KLEL_ASSERT(gpcKlelQuotedChars != NULL);

  return gpcKlelQuotedChars;
}


/*-
 ***********************************************************************
 *
 * KlelGetRelease
 *
 ***********************************************************************
 */
char *
KlelGetRelease(void)
{
  return PACKAGE_VERSION;
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
 * KlelGetTypeOfExpression
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelGetTypeOfExpression(KLEL_NODE *psExpression, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psExpression                                           != NULL);
  KLEL_ASSERT(psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] != NULL);

  if (psExpression == NULL || psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] == NULL)
  {
    return KLEL_EXPR_UNKNOWN;
  }

  return KlelTypeCheck(psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX], psContext);
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
 * KlelIsGuardedCommand
 *
 ***********************************************************************
 */
int
KlelIsGuardedCommand(KLEL_NODE *psExpression)
{
  KLEL_ASSERT(psExpression != NULL);

  if (psExpression != NULL)
  {
    if (strcmp(psExpression->acName, "root") == 0)
    {
      psExpression = psExpression->apsChildren[KLEL_EXPRESSION_CHILD_INDEX];
    }
  }

  return (psExpression != NULL && strcmp(psExpression->acName, "guard") == 0);
}


/*-
 ***********************************************************************
 *
 * KlelIsSuccessReturnCode
 *
 ***********************************************************************
 */
int
KlelIsSuccessReturnCode(KLEL_NODE *psRoot, int code)
{
  size_t szi = 0;

  KLEL_ASSERT(KlelIsGuardedCommand(psRoot));

  if (KlelIsGuardedCommand(psRoot))
  {
    psRoot = psRoot->apsChildren[KLEL_EXPRESSION_CHILD_INDEX];

    if (psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX] == NULL && psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX] == NULL)
    {
      return (code == 0); /* If no codes are specified, zero is considered success by default. */
    }

    if (psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX] != NULL)
    {
      for (szi = 0; szi < KLEL_MAX_CHILDREN && psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX]->apsChildren[szi] != NULL; szi++)
      {
        if (code == psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX]->apsChildren[szi]->llInteger)
        {
          return 1;
        }
      }
    }

    if (psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX] != NULL)
    {
      for (szi = 0; szi < KLEL_MAX_CHILDREN && psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX]->apsChildren[szi] != NULL; szi++)
      {
        if (code == psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX]->apsChildren[szi]->llInteger)
        {
          return 0;
        }
      }

      return 1;
    }
  }

  return 0;
}


/*-
 ***********************************************************************
 *
 * KlelSetQuoteChar
 *
 ***********************************************************************
 */
void
KlelSetQuoteChar(char c)
{
  KLEL_ASSERT(!isalnum(c) && !isspace(c) && isprint(c));

  if (!isalnum(c) && !isspace(c) && isprint(c))
  {
    gcKlelQuoteChar = c;
  }
}


/*-
 ***********************************************************************
 *
 * KlelSetQuotedChars
 *
 ***********************************************************************
 */
void
KlelSetQuotedChars(char *c)
{
  KLEL_ASSERT(c != NULL);

  if (c != NULL)
  {
    gpcKlelQuotedChars = c;
  }
}


/*-
 ***********************************************************************
 *
 * KlelValueToQuotedString
 *
 ***********************************************************************
 */
char *
KlelValueToQuotedString(KLEL_VALUE *psValue, size_t *szLength)
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
KlelValueToString(KLEL_VALUE *psValue, size_t *szLength)
{
  char *pcBuffer = calloc(1, KLEL_MAX_NAME + 1);

  KLEL_ASSERT(psValue  != NULL);
  KLEL_ASSERT(szLength != NULL);

  if (pcBuffer != NULL)
  {
    switch (psValue->iType)
    {
      case KLEL_EXPR_BOOLEAN:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%s", psValue->bBoolean ? "true" : "false");
        *szLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_FLOAT:
        snprintf(pcBuffer, KLEL_MAX_NAME, "%g", psValue->dFloat);
        *szLength = strlen(pcBuffer);
        break;

      case KLEL_EXPR_FUNCTION_BOOLEAN:
      case KLEL_EXPR_FUNCTION_FLOAT:
      case KLEL_EXPR_FUNCTION_INTEGER:
      case KLEL_EXPR_FUNCTION_STRING:
        snprintf(pcBuffer, KLEL_MAX_NAME, "\\%s", psValue->acString);
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
        KLEL_ASSERT(0); /* Data structure corruption. */
        return NULL;
    }
  }

  return pcBuffer;
}
