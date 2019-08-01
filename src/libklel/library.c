/*-
 ***********************************************************************
 *
 * $Id: library.c,v 1.41 2019/07/31 15:59:27 klm Exp $
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
 * Standard library entries.
 *
 ***********************************************************************
 */
static KLEL_STANDARD_LIBRARY_ENTRY gasKlelStdLib[] =
{
  /*-
   *********************************************************************
   *
   * Boolean Values
   *
   *********************************************************************
   */

  {"true",  KLEL_TYPE_BOOLEAN, 1},
  {"false", KLEL_TYPE_BOOLEAN, 0},

  /*-
   *********************************************************************
   *
   * Numeric Values
   *
   *********************************************************************
   */

  {"pi", KLEL_TYPE_REAL, 0, 3.1415926535897932384},
  {"e",  KLEL_TYPE_REAL, 0, 2.7182818284590452354},

  /*-
   *********************************************************************
   *
   * Arithmetic Functions
   *
   *********************************************************************
   */

  {"abs",             KLEL_TYPE_INT64_FUNCTION1(KLEL_TYPE_INT64),                  0, 0, KlelStdLibAbs},
  {"entier",          KLEL_TYPE_INT64_FUNCTION1(KLEL_TYPE_REAL),                   0, 0, KlelStdLibEntier},
  {"real",            KLEL_TYPE_REAL_FUNCTION1(KLEL_TYPE_INT64),                   0, 0, KlelStdLibReal},
  {"max",             KLEL_TYPE_INT64_FUNCTION2(KLEL_TYPE_INT64, KLEL_TYPE_INT64), 0, 0, KlelStdLibMax},
  {"min",             KLEL_TYPE_INT64_FUNCTION2(KLEL_TYPE_INT64, KLEL_TYPE_INT64), 0, 0, KlelStdLibMin},
  {"odd",             KLEL_TYPE_BOOLEAN_FUNCTION1(KLEL_TYPE_INT64),                0, 0, KlelStdLibOdd},

  /*-
   *********************************************************************
   *
   * System Functions
   *
   *********************************************************************
   */

  {"abort",           KLEL_TYPE_BOOLEAN_FUNCTION1(KLEL_TYPE_STRING),                   0, 0, KlelStdLibAbort},
  {"abort_bool",      KLEL_TYPE_BOOLEAN_FUNCTION1(KLEL_TYPE_STRING),                   0, 0, KlelStdLibAbort},
  {"abort_int",       KLEL_TYPE_INT64_FUNCTION1(KLEL_TYPE_STRING),                     0, 0, KlelStdLibAbort},
  {"abort_real",      KLEL_TYPE_REAL_FUNCTION1(KLEL_TYPE_STRING),                      0, 0, KlelStdLibAbort},
  {"abort_string",    KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_STRING),                    0, 0, KlelStdLibAbort},

  /*-
   *********************************************************************
   *
   * String Functions
   *
   *********************************************************************
   */

  {"cksum",           KLEL_TYPE_INT64_FUNCTION1(KLEL_TYPE_STRING),                     0, 0, KlelStdLibCksum},
  {"chr",             KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_INT64),                     0, 0, KlelStdLibChr},
  {"ord",             KLEL_TYPE_INT64_FUNCTION1(KLEL_TYPE_STRING),                     0, 0, KlelStdLibOrd},
  {"strlen",          KLEL_TYPE_INT64_FUNCTION1(KLEL_TYPE_STRING),                     0, 0, KlelStdLibStrlen},
  {"string_of_bool",  KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_BOOLEAN),                   0, 0, KlelStdLibStringOfBool},
  {"string_of_real",  KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_REAL),                      0, 0, KlelStdLibStringOfReal},
  {"string_of_int",   KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_INT64),                     0, 0, KlelStdLibStringOfInt},
  {"dec_of_int",      KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_INT64),                     0, 0, KlelStdLibStringOfInt},
  {"oct_of_int",      KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_INT64),                     0, 0, KlelStdLibOctOfInt},
  {"hex_of_int",      KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_INT64),                     0, 0, KlelStdLibHexOfInt},

  /*-
   *********************************************************************
   *
   * Time Functions
   *
   *********************************************************************
   */

  {"now",             KLEL_TYPE_INT64_FUNCTION0(),                                     0, 0, KlelStdLibNow},
  {"ctime",           KLEL_TYPE_STRING_FUNCTION0(),                                    0, 0, KlelStdLibCtime},
};


/*-
 ***********************************************************************
 *
 * KlelStdLibAbs
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibAbort(KLEL_VALUE **asArgs, void *pvContext)
{
  KlelReportError(pvContext, "aborted: %s", KlelValueToString(asArgs[0], pvContext), NULL);

  return NULL;
}

/*-
 ***********************************************************************
 *
 * KlelStdLibAbs
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibAbs(KLEL_VALUE **asArgs, void *pvContext)
{
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
KlelStdLibCksum(KLEL_VALUE **asArgs, void *pvContext)
{
  size_t   szi  = 0;
  uint32_t iSum = 0;

  /*-
   ***********************************************************************
   *
   * This is the "standard" original BSD checksum algorithm.
   *
   ***********************************************************************
   */

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
KlelStdLibChr(KLEL_VALUE **asArgs, void *pvContext)
{
  char acString[2] = {0};

  acString[0] = (char)(asArgs[0]->llInteger);
  acString[1] = 0;

  return KlelCreateString(1, acString);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibCtime
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibCtime(KLEL_VALUE **asArgs, void *pvContext)
{
  char   acBuf[100] = {0};
  time_t tNow       = time(NULL);

  strncpy(acBuf, ctime(&tNow), sizeof(acBuf) - 1);

  return KlelCreateString(strlen(acBuf), acBuf);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibEntier
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibEntier(KLEL_VALUE **asArgs, void *pvContext)
{
  return KlelCreateInteger(asArgs[0]->dReal);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibReal
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibReal(KLEL_VALUE **asArgs, void *pvContext)
{
  return KlelCreateReal(asArgs[0]->llInteger * 1.0);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibMax
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibMax(KLEL_VALUE **asArgs, void *pvContext)
{
  return KlelCreateInteger((asArgs[0]->llInteger >= asArgs[1]->llInteger) ? asArgs[0]->llInteger : asArgs[1]->llInteger);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibMin
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibMin(KLEL_VALUE **asArgs, void *pvContext)
{
  return KlelCreateInteger((asArgs[0]->llInteger <= asArgs[1]->llInteger) ? asArgs[0]->llInteger : asArgs[1]->llInteger);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibNow
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibNow(KLEL_VALUE **asArgs, void *pvContext)
{
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
KlelStdLibOdd(KLEL_VALUE **asArgs, void *pvContext)
{
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
KlelStdLibOrd(KLEL_VALUE **asArgs, void *pvContext)
{
  return KlelCreateInteger(asArgs[0]->szLength == 0 ? 0 : asArgs[0]->acString[0]);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibStrlen
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStrlen(KLEL_VALUE **asArgs, void *pvContext)
{
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
KlelStdLibStringOfBool(KLEL_VALUE **asArgs, void *pvContext)
{
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
 * KlelStdLibStringOfReal
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStringOfReal(KLEL_VALUE **asArgs, void *pvContext)
{
  char acBuf[100] = {0};

  snprintf(acBuf, sizeof(acBuf) - 1, "%g", asArgs[0]->dReal);

  return KlelCreateString(strlen(acBuf), acBuf);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibStringOfInt
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibStringOfInt(KLEL_VALUE **asArgs, void *pvContext)
{
  char acBuf[100] = {0};

  snprintf(acBuf, sizeof(acBuf) - 1, "%" PRId64, asArgs[0]->llInteger);

  return KlelCreateString(strlen(acBuf), acBuf);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibHexOfInt
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibHexOfInt(KLEL_VALUE **asArgs, void *pvContext)
{
  char acBuf[100] = {0};

  snprintf(acBuf, sizeof(acBuf) - 1, "%" PRIx64, asArgs[0]->llInteger);

  return KlelCreateString(strlen(acBuf), acBuf);
}


/*-
 ***********************************************************************
 *
 * KlelStdLibOctOfInt
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelStdLibOctOfInt(KLEL_VALUE **asArgs, void *pvContext)
{
  char acBuf[100] = {0};

  snprintf(acBuf, sizeof(acBuf) - 1, "%" PRIo64, asArgs[0]->llInteger);

  return KlelCreateString(strlen(acBuf), acBuf);
}


/*-
 ***********************************************************************
 *
 * KlelGetTypeOfStdVar (utility function)
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
KlelGetTypeOfStdVar(const char *pcName, void *pvContext)
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

  return KLEL_TYPE_UNKNOWN;
}


/*-
 ***********************************************************************
 *
 * KlelGetValueOfStdVar (utility function)
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelGetValueOfStdVar(const char *pcName, void *pvContext)
{
  size_t szi = 0;

  KLEL_ASSERT(pcName != NULL);

  for (szi = 0; szi < sizeof(gasKlelStdLib) / sizeof(KLEL_STANDARD_LIBRARY_ENTRY); szi++)
  {
    if (strcmp(gasKlelStdLib[szi].pcName, pcName) == 0)
    {
      switch (gasKlelStdLib[szi].iType)
      {
        case KLEL_TYPE_BOOLEAN:
          return KlelCreateBoolean(gasKlelStdLib[szi].bBoolean);

        case KLEL_TYPE_REAL:
          return KlelCreateReal(gasKlelStdLib[szi].dReal);

        case KLEL_TYPE_INT64:
          return KlelCreateInteger(gasKlelStdLib[szi].llInteger);

        case KLEL_TYPE_STRING:
          return KlelCreateString(strlen(gasKlelStdLib[szi].pcString), gasKlelStdLib[szi].pcString);

        default:
          return KlelCreateValue(gasKlelStdLib[szi].iType, gasKlelStdLib[szi].pcName, gasKlelStdLib[szi].fFunction);
      }
    }
  }

  return NULL;
}
