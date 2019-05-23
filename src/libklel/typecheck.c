/*-
 ***********************************************************************
 *
 * $Id: typecheck.c,v 1.29 2012/04/26 00:53:31 klm Exp $
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
 * Type checking rules.
 *
 ***********************************************************************
 */
static KLEL_TYPE_DESC gasKlelTypes[] =
{
  {"float",   NULL,   {0},                                        KLEL_EXPR_FLOAT},
  {"int",     NULL,   {0},                                        KLEL_EXPR_INTEGER},

  {"cond",    NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_FLOAT,   KLEL_EXPR_FLOAT},   KLEL_EXPR_FLOAT},
  {"cond",    NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER}, KLEL_EXPR_INTEGER},
  {"cond",    NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_BOOLEAN, KLEL_EXPR_BOOLEAN}, KLEL_EXPR_BOOLEAN},
  {"cond",    NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_STRING,  KLEL_EXPR_STRING},  KLEL_EXPR_STRING},
  {"cond",    NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_FLOAT,   KLEL_EXPR_FLOAT},   KLEL_EXPR_FLOAT},
  {"cond",    NULL,   {0}, KLEL_EXPR_UNKNOWN, "conditionals must have a boolean predicate and identical child types"},

  {"+",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_FLOAT},
  {"+",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_INTEGER},                 KLEL_EXPR_FLOAT},
  {"+",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_FLOAT},                 KLEL_EXPR_FLOAT},
  {"+",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"+",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'+' is only applicable to numeric types"},

  {"-",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_FLOAT},
  {"-",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_INTEGER},                 KLEL_EXPR_FLOAT},
  {"-",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_FLOAT},                 KLEL_EXPR_FLOAT},
  {"-",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"-",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'-' is only applicable to numeric types"},

  {"*",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_FLOAT},
  {"*",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_INTEGER},                 KLEL_EXPR_FLOAT},
  {"*",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_FLOAT},                 KLEL_EXPR_FLOAT},
  {"*",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"*",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'*' is only applicable to numeric types"},

  {"/",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_FLOAT},
  {"/",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_INTEGER},                 KLEL_EXPR_FLOAT},
  {"/",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_FLOAT},                 KLEL_EXPR_FLOAT},
  {"/",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"/",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'/' is only applicable to numeric types"},

  {".",       NULL,   {KLEL_EXPR_STRING, KLEL_EXPR_STRING},                 KLEL_EXPR_STRING},
  {".",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'.' is only applicable to string types"},

  {"%",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"^",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"|",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"<<",      NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {">>",      NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"&",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_INTEGER},
  {"%",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'%' is only applicable to integer types"},
  {"^",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'^' is only applicable to integer types"},
  {"|",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'|' is only applicable to integer types"},
  {"&",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'&' is only applicable to integer types"},
  {">>",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'>>' is only applicable to integer types"},
  {"<<",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'<<' is only applicable to integer types"},

  {"<",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_BOOLEAN},
  {"<",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_BOOLEAN},
  {"<=",      NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_BOOLEAN},
  {"<=",      NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_BOOLEAN},
  {">",       NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_BOOLEAN},
  {">",       NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_BOOLEAN},
  {">=",      NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_BOOLEAN},
  {">=",      NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_BOOLEAN},
  {"<",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'<' is only applicable to numbers of the same type"},
  {"<=",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'<=' is only applicable to numbers of the same type"},
  {">",       NULL,   {0}, KLEL_EXPR_UNKNOWN, "'>' is only applicable to numbers of the same type"},
  {">=",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'>=' is only applicable to numbers of the same type"},

  {"==",      NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_BOOLEAN},               KLEL_EXPR_BOOLEAN},
  {"==",      NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_BOOLEAN},
  {"==",      NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_BOOLEAN},
  {"==",      NULL,   {KLEL_EXPR_STRING, KLEL_EXPR_STRING},                 KLEL_EXPR_BOOLEAN},
  {"==",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'==' is only applicable to two arguments of the same type"},

  {"!=",      NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_BOOLEAN},               KLEL_EXPR_BOOLEAN},
  {"!=",      NULL,   {KLEL_EXPR_FLOAT, KLEL_EXPR_FLOAT},                   KLEL_EXPR_BOOLEAN},
  {"!=",      NULL,   {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER},               KLEL_EXPR_BOOLEAN},
  {"!=",      NULL,   {KLEL_EXPR_STRING, KLEL_EXPR_STRING},                 KLEL_EXPR_BOOLEAN},
  {"!=",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'!=' is only applicable to two arguments of the same type"},

  {"=~",      NULL,   {KLEL_EXPR_STRING, KLEL_EXPR_STRING},                 KLEL_EXPR_BOOLEAN},
  {"!~",      NULL,   {KLEL_EXPR_STRING, KLEL_EXPR_STRING},                 KLEL_EXPR_BOOLEAN},
  {"=~",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'=~' is only applicable to strings"},
  {"!~",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'!~' is only applicable to strings"},

  {"&&",      NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_BOOLEAN},               KLEL_EXPR_BOOLEAN},
  {"||",      NULL,   {KLEL_EXPR_BOOLEAN, KLEL_EXPR_BOOLEAN},               KLEL_EXPR_BOOLEAN},
  {"&&",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'&&' is only applicable to boolean values"},
  {"||",      NULL,   {0}, KLEL_EXPR_UNKNOWN, "'||' is only applicable to boolean values"},

  {"bnot",    NULL,   {KLEL_EXPR_INTEGER},                             KLEL_EXPR_INTEGER},
  {"bnot",    NULL,   {0}, KLEL_EXPR_UNKNOWN, "'~' is only applicable to integer values"},

  {"neg",     NULL,   {KLEL_EXPR_INTEGER},                             KLEL_EXPR_INTEGER},
  {"neg",     NULL,   {KLEL_EXPR_FLOAT},                               KLEL_EXPR_FLOAT},
  {"neg",     NULL,   {0}, KLEL_EXPR_UNKNOWN, "'-' is only applicable to numeric values"},

  {"not",     NULL,   {KLEL_EXPR_BOOLEAN},                             KLEL_EXPR_BOOLEAN},
  {"not",     NULL,   {0}, KLEL_EXPR_UNKNOWN, "'!' is only applicable to boolean values"},

  {"call",   KlelTypeCheckCall},
  {"guard",  KlelTypeCheckGuard},
  {"quote",  KlelTypeCheckVar},
  {"string", KlelTypeCheckString},
  {"var",    KlelTypeCheckVar}
};

/*-
 ***********************************************************************
 *
 * KlelTypeCheck
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheck(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  int    bSuccess = 0;
  size_t szi      = 0;
  size_t szj      = 0;

  KLEL_ASSERT(psRoot != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "root") != 0 || psRoot->apsChildren[KLEL_EXPRESSION_CHILD_INDEX] != NULL);

  if (strcmp(psRoot->acName, "root") == 0)
  {
    psRoot = psRoot->apsChildren[KLEL_EXPRESSION_CHILD_INDEX];
  }

  for (szi = 0; szi < sizeof(gasKlelTypes) / sizeof(KLEL_TYPE_DESC); szi++)
  {
    KlelClearErrors(psContext);
    if (strcmp(gasKlelTypes[szi].pcName, psRoot->acName) == 0)
    {
      if (gasKlelTypes[szi].pcError != NULL)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, gasKlelTypes[szi].pcError, psRoot->pcInput, NULL);
        return KLEL_EXPR_UNKNOWN;
      }

      if (*(gasKlelTypes[szi].fFunc) != NULL)
      {
        return (*(gasKlelTypes[szi].fFunc))(psRoot, psContext);
      }

      bSuccess = 1;

      for (szj = 0; szj < KLEL_MAX_CHILDREN; szj++)
      {
        if (gasKlelTypes[szi].aiChildren[szj] == KLEL_EXPR_UNKNOWN)
        {
          if (psRoot->apsChildren[szj] == NULL)
          {
            return gasKlelTypes[szi].iType;
          }
        }

        if
        (
          KlelTypeCheck(psRoot->apsChildren[szj], psContext) != gasKlelTypes[szi].aiChildren[szj]
        )
        {
          bSuccess = 0;
          break;
        }
      }

      if (bSuccess)
      {
        return gasKlelTypes[szi].iType;
      }
    }
  }

  KLEL_ASSERT(0);
  return KLEL_EXPR_UNKNOWN;
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
  KLEL_EXPR_TYPE iType  = KLEL_EXPR_UNKNOWN;
  KLEL_EXPR_TYPE *aiArg = NULL;
  size_t         szi    = 0;

  KLEL_ASSERT(psRoot                 != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0] != NULL);

  iType = KlelTypeCheck(psRoot->apsChildren[0], psContext);
  if (iType != KLEL_EXPR_FUNCTION_BOOLEAN && iType != KLEL_EXPR_FUNCTION_FLOAT && iType != KLEL_EXPR_FUNCTION_INTEGER && iType != KLEL_EXPR_FUNCTION_STRING)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "called value is not a function", psRoot->pcInput, NULL);
    return KLEL_EXPR_UNKNOWN;
  }

  aiArg = KlelGetFuncDesc(psRoot->apsChildren[0]->acString, psContext);
  if (aiArg == NULL)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unable to get type information for function", psRoot->pcInput, NULL);
    return KLEL_EXPR_UNKNOWN;
  }

  if (psRoot->apsChildren[1] == NULL && aiArg[0] != KLEL_EXPR_UNKNOWN)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "insufficient arguments for function", psRoot->pcInput, NULL);
    free(aiArg);
    return KLEL_EXPR_UNKNOWN;
  }

  if (psRoot->apsChildren[1] != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS; szi++)
    {
      if (aiArg[szi] == KLEL_EXPR_UNKNOWN && psRoot->apsChildren[1]->apsChildren[szi] != NULL)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "incorrect number of arguments", psRoot->pcInput, NULL);
        free(aiArg);
        return KLEL_EXPR_UNKNOWN;
      }

      if (psRoot->apsChildren[1]->apsChildren[szi] == NULL && aiArg[szi] != KLEL_EXPR_UNKNOWN)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "incorrect number of arguments", psRoot->pcInput, NULL);
        free(aiArg);
        return KLEL_EXPR_UNKNOWN;
      }

      if (aiArg[szi] == KLEL_EXPR_UNKNOWN)
      {
        break;
      }

      if (KlelTypeCheck(psRoot->apsChildren[1]->apsChildren[szi], psContext) != aiArg[szi])
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "incorrect type of arguments", psRoot->pcInput, NULL);
        free(aiArg);
        return KLEL_EXPR_UNKNOWN;
      }
    }
  }

  free(aiArg);

  switch (iType)
  {
    case KLEL_EXPR_FUNCTION_BOOLEAN:
      return KLEL_EXPR_BOOLEAN;

    case KLEL_EXPR_FUNCTION_FLOAT:
      return KLEL_EXPR_FLOAT;

    case KLEL_EXPR_FUNCTION_INTEGER:
      return KLEL_EXPR_INTEGER;

    case KLEL_EXPR_FUNCTION_STRING:
      return KLEL_EXPR_STRING;

    default:
      KLEL_ASSERT(0);
      break;
  }

  return KLEL_EXPR_UNKNOWN;
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckGuard
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckGuard(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  size_t szi = 0;

  KLEL_ASSERT(psRoot                 != NULL);
  KLEL_ASSERT(psRoot->apsChildren[0] != NULL);

  if (KlelTypeCheck(psRoot->apsChildren[KLEL_GUARD_CHILD_INDEX], psContext) != KLEL_EXPR_BOOLEAN)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "guarded commands must have a boolean predicate", psRoot->pcInput, NULL);
    return KLEL_EXPR_UNKNOWN;
  }

  if (psRoot->apsChildren[KLEL_COMMAND_CHILD_INDEX] != NULL)
  {
    if (strcmp(psRoot->apsChildren[KLEL_COMMAND_CHILD_INDEX]->acName, "args") == 0)
    {
      for (szi = 0; szi < KLEL_MAX_FUNC_ARGS && psRoot->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[szi] != NULL; szi++)
      {
        if (KlelTypeCheck(psRoot->apsChildren[KLEL_COMMAND_CHILD_INDEX]->apsChildren[szi], psContext) == KLEL_EXPR_UNKNOWN)
        {
          return KLEL_EXPR_UNKNOWN;
        }
      }
    }
    else if (strcmp(psRoot->apsChildren[KLEL_COMMAND_CHILD_INDEX]->acName, "string") == 0)
    {
      if (KlelTypeCheck(psRoot->apsChildren[KLEL_COMMAND_CHILD_INDEX], psContext) == KLEL_EXPR_UNKNOWN)
      {
        return KLEL_EXPR_UNKNOWN;
      }
    }
  }

  if (psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX] != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS && psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX]->apsChildren[szi] != NULL; szi++)
    {
      if (KlelTypeCheck(psRoot->apsChildren[KLEL_SUCCESS_CHILD_INDEX]->apsChildren[szi], psContext) != KLEL_EXPR_INTEGER)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "return codes must be of type integer", psRoot->pcInput, NULL);
        return KLEL_EXPR_UNKNOWN;
      }
    }
  }

  if (psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX] != NULL)
  {
    for (szi = 0; szi < KLEL_MAX_FUNC_ARGS && psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX]->apsChildren[szi] != NULL; szi++)
    {
      if (KlelTypeCheck(psRoot->apsChildren[KLEL_FAILURE_CHILD_INDEX]->apsChildren[szi], psContext) != KLEL_EXPR_INTEGER)
      {
        KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "return codes must be of type integer", psRoot->pcInput, NULL);
        return KLEL_EXPR_UNKNOWN;
      }
    }
  }

  return KLEL_EXPR_BOOLEAN;
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckString
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckString(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  size_t szi = 0;

  KLEL_ASSERT(psRoot                           != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "string") == 0);

  for (szi = 0; szi < KLEL_MAX_CHILDREN; szi++)
  {
    if (psRoot->apsChildren[szi] == NULL)
    {
      break;
    }

    if (strcmp(psRoot->apsChildren[szi]->acName, "fragment") == 0)
    {
      continue;
    }

    if (strcmp(psRoot->apsChildren[szi]->acName, "char") == 0)
    {
      continue;
    }

    KLEL_ASSERT(strcmp(psRoot->apsChildren[szi]->acName, "var")   == 0 || strcmp(psRoot->apsChildren[szi]->acName, "quote") == 0);

    if (KlelTypeCheck(psRoot->apsChildren[szi], psContext) == KLEL_EXPR_UNKNOWN)
    {
      KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "invalid string interpolation", psRoot->pcInput, NULL);
      return KLEL_EXPR_UNKNOWN;
    }
  }

  return KLEL_EXPR_STRING;
}


/*-
 ***********************************************************************
 *
 * KlelTypeCheckVar
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelTypeCheckVar(KLEL_NODE *psRoot, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(psRoot           != NULL);
  KLEL_ASSERT(KlelGetTypeOfVar != NULL);
  KLEL_ASSERT(KlelReportError  != NULL);
  KLEL_ASSERT(strcmp(psRoot->acName, "var")   == 0 || strcmp(psRoot->acName, "quote") == 0);

  if (KlelGetTypeOfVar(psRoot->acString, psContext) == KLEL_EXPR_UNKNOWN)
  {
    KlelReportError(psContext, KLEL_COMPILE_ERROR_FORMAT, "unknown variable", psRoot->acString, NULL);
    return KLEL_EXPR_UNKNOWN;
  }

  return KlelGetTypeOfVar(psRoot->acString, psContext);
}
