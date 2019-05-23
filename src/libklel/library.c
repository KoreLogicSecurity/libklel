/*-
 ***********************************************************************
 *
 * $Id: library.c,v 1.30 2012/04/23 16:40:22 klm Exp $
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
 * Standard library entries.
 *
 ***********************************************************************
 */
static KLEL_STANDARD_LIBRARY_ENTRY gasKlelStdLib[] =
{
  /* Boolean values. */
  {"true",    KLEL_EXPR_BOOLEAN, 1},
  {"false",   KLEL_EXPR_BOOLEAN, 0},

  /* Numeric values. */
  {"pi",       KLEL_EXPR_FLOAT,   0, 3.1415926535897932384},
  {"e",        KLEL_EXPR_FLOAT,   0, 2.7182818284590452354},
  {"int_bits", KLEL_EXPR_INTEGER, 0, 0, NULL, sizeof(int) * CHAR_BIT},

  /* Numeric functions. */
  {"abs",             KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibAbs,           0, NULL, {KLEL_EXPR_INTEGER}},
  {"entier",          KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibEntier,        0, NULL, {KLEL_EXPR_FLOAT}},
  {"float",           KLEL_EXPR_FUNCTION_FLOAT,   0, 0, KlelStdLibFloat,         0, NULL, {KLEL_EXPR_INTEGER}},
  {"max",             KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibMax,           0, NULL, {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER}},
  {"min",             KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibMin,           0, NULL, {KLEL_EXPR_INTEGER, KLEL_EXPR_INTEGER}},
  {"odd",             KLEL_EXPR_FUNCTION_BOOLEAN, 0, 0, KlelStdLibOdd,           0, NULL, {KLEL_EXPR_INTEGER}},

  /* String functions. */
  {"cksum",           KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibCksum,         0, NULL, {KLEL_EXPR_STRING}},
  {"chr",             KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibChr,           0, NULL, {KLEL_EXPR_INTEGER}},
  {"ord",             KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibOrd,           0, NULL, {KLEL_EXPR_STRING}},
  {"strlen",          KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibStrlen,        0, NULL, {KLEL_EXPR_STRING}},
  {"string_of_bool",  KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibStringOfBool,  0, NULL, {KLEL_EXPR_BOOLEAN}},
  {"string_of_float", KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibStringOfFloat, 0, NULL, {KLEL_EXPR_FLOAT}},
  {"string_of_int",   KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibStringOfInt,   0, NULL, {KLEL_EXPR_INTEGER}},
  {"dec_of_int",      KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibStringOfInt,   0, NULL, {KLEL_EXPR_INTEGER}},
  {"oct_of_int",      KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibOctOfInt,      0, NULL, {KLEL_EXPR_INTEGER}},
  {"hex_of_int",      KLEL_EXPR_FUNCTION_STRING,  0, 0, KlelStdLibHexOfInt,      0, NULL, {KLEL_EXPR_INTEGER}},

  /* Time functions. */
  {"now",             KLEL_EXPR_FUNCTION_INTEGER, 0, 0, KlelStdLibNow,           0, NULL, {0}}
};

/*-
 ***********************************************************************
 *
 * KlelStdLibAbs
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibAbs(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  return KlelCreateInteger(asArgs[0]->llInteger < 0 ? -(asArgs[0]->llInteger) : asArgs[0]->llInteger);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibCksum
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibCksum(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  size_t   szi  = 0;
  uint32_t iSum = 0;

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_STRING);

  /* This is the "standard" original BSD checksum algorithm. */
  for (szi = 0; szi < asArgs[0]->szLength; szi++)
  {
    iSum  = (iSum >> 1) + ((iSum & 1) << 15);
    iSum += asArgs[0]->acString[szi];
    iSum &= 0xFFFF;
  }

  return KlelCreateInteger(iSum);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibChr
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibChr(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  char acString[2] = {0};

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  acString[0] = (char)(asArgs[0]->llInteger);
  acString[1] = 0;

  return KlelCreateString(1, acString);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibEntier
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibEntier(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_FLOAT);

  return KlelCreateInteger(asArgs[0]->dFloat);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibFloat
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibFloat(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  return KlelCreateFloat(asArgs[0]->llInteger * 1.0);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibMax
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibMax(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        != NULL);
  KLEL_ASSERT(asArgs[2]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(asArgs[1]->iType == KLEL_EXPR_INTEGER);

  if (asArgs[0]->llInteger >= asArgs[1]->llInteger)
  {
    return KlelCreateInteger(asArgs[0]->llInteger);
  }

  return KlelCreateInteger(asArgs[1]->llInteger);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibMin
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibMin(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        != NULL);
  KLEL_ASSERT(asArgs[2]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);
  KLEL_ASSERT(asArgs[1]->iType == KLEL_EXPR_INTEGER);

  if (asArgs[0]->llInteger <= asArgs[1]->llInteger)
  {
    return KlelCreateInteger(asArgs[0]->llInteger);
  }

  return KlelCreateInteger(asArgs[1]->llInteger);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibNow
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibNow(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs == NULL || asArgs[0] == NULL);

  return KlelCreateInteger(time(NULL));
}


/*-
 ***********************************************************************
 *
 * KlelStdLibOdd
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibOdd(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  return KlelCreateBoolean(asArgs[0]->llInteger % 2 == 1);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibOrd
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibOrd(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_STRING);

  if (asArgs[0]->szLength == 0)
  {
    return KlelCreateInteger(0);
  }

  return KlelCreateInteger(asArgs[0]->acString[0]);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibStrlen
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStrlen(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_STRING);

  return KlelCreateInteger(asArgs[0]->szLength);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibStringOfBool
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStringOfBool(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_BOOLEAN);

  if (asArgs[0]->bBoolean)
  {
    return KlelCreateString(sizeof("true"), "true");
  }
  else
  {
    return KlelCreateString(sizeof("false"), "false");
  }
}


/*-
 ***********************************************************************
 *
 * KlelStdLibStringOfFloat
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStringOfFloat(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  char buf[100] = {0};

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_FLOAT);

  snprintf(buf, sizeof(buf) - 1, "%g", asArgs[0]->dFloat);

  return KlelCreateString(strlen(buf), buf);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibStringOfInt
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStringOfInt(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  char buf[100] = {0};

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  snprintf(buf, sizeof(buf) - 1, "%" PRId64, asArgs[0]->llInteger);

  return KlelCreateString(strlen(buf), buf);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibHexOfInt
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibHexOfInt(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  char buf[100] = {0};

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  snprintf(buf, sizeof(buf) - 1, "%" PRIx64, asArgs[0]->llInteger);

  return KlelCreateString(strlen(buf), buf);
}

/*-
 ***********************************************************************
 *
 * KlelStdLibOctOfInt
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibOctOfInt(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  char buf[100] = {0};

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_INTEGER);

  snprintf(buf, sizeof(buf) - 1, "%" PRIo64, asArgs[0]->llInteger);

  return KlelCreateString(strlen(buf), buf);
}


/*-
 ***********************************************************************
 *
 * KlelGetStdLibFuncDesc (utility function)
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE *
KlelGetStdLibFuncDesc(const char *pcName, KLEL_CONTEXT *psContext)
{
  size_t         szi       = 0;
  size_t         szj       = 0;
  KLEL_EXPR_TYPE *aiResult = calloc(KLEL_MAX_FUNC_ARGS, sizeof(KLEL_EXPR_TYPE));

  KLEL_ASSERT(pcName != NULL);

  if (aiResult != NULL)
  {
    for (szi = 0; szi < sizeof(gasKlelStdLib) / sizeof(KLEL_STANDARD_LIBRARY_ENTRY); szi++)
    {
      if (strcmp(gasKlelStdLib[szi].pcName, pcName) == 0)
      {
        for (szj = 0; szj < KLEL_MAX_FUNC_ARGS; szj++)
        {
          aiResult[szj] = gasKlelStdLib[szi].aiArguments[szj];
        }
        return aiResult;
      }
    }
  }

  return NULL;
}


/*-
 ***********************************************************************
 *
 * KlelGetTypeOfStdVar (utility function)
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelGetTypeOfStdVar(const char *pcName, KLEL_CONTEXT *psContext)
{
  size_t szi = 0;

  KLEL_ASSERT(pcName != NULL);

  for (szi = 0; szi < sizeof(gasKlelStdLib) / sizeof(KLEL_STANDARD_LIBRARY_ENTRY); szi++)
  {
    if (strcmp(gasKlelStdLib[szi].pcName, pcName) == 0)
    {
      return gasKlelStdLib[szi].iType;
    }
  }

  return KLEL_EXPR_UNKNOWN;
}


/*-
 ***********************************************************************
 *
 * KlelGetValueOfStdVar (utility function)
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelGetValueOfStdVar(const char *pcName, KLEL_CONTEXT *psContext)
{
  size_t szi = 0;

  KLEL_ASSERT(pcName != NULL);

  for (szi = 0; szi < sizeof(gasKlelStdLib) / sizeof(KLEL_STANDARD_LIBRARY_ENTRY); szi++)
  {
    if (strcmp(gasKlelStdLib[szi].pcName, pcName) == 0)
    {
      switch (gasKlelStdLib[szi].iType)
      {
        case KLEL_EXPR_BOOLEAN:
          return KlelCreateBoolean(gasKlelStdLib[szi].bBoolean);

        case KLEL_EXPR_FLOAT:
          return KlelCreateFloat(gasKlelStdLib[szi].dFloat);

        case KLEL_EXPR_FUNCTION_BOOLEAN:
        case KLEL_EXPR_FUNCTION_FLOAT:
        case KLEL_EXPR_FUNCTION_INTEGER:
        case KLEL_EXPR_FUNCTION_STRING:
          return KlelCreateValue(gasKlelStdLib[szi].iType, gasKlelStdLib[szi].pcName, gasKlelStdLib[szi].fFunction);

        case KLEL_EXPR_INTEGER:
          return KlelCreateInteger(gasKlelStdLib[szi].llInteger);

        case KLEL_EXPR_STRING:
          return KlelCreateString(strlen(gasKlelStdLib[szi].pcString), gasKlelStdLib[szi].pcString);

        default:
          KLEL_ASSERT(0);
          return NULL;
      }
    }
  }

  return NULL;
}
