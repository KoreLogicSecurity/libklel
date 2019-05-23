/*-
 ***********************************************************************
 *
 * $Id: tokenizer.c,v 1.19 2012/11/14 02:42:06 klm Exp $
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
 * Tokenization Patterns
 * This table lists the tokenization patterns; these patterns indicate
 * patterns in input that, when matched, are transformed into tokens
 * that can then be passed to the parser.
 *
 * The structure of these patterns may seem complex, but it's actually
 * fairly simple. The entry structure has the format:
 *
 * KLEL_TOKENIZATION_MODE iMode
 * KLEL_TOKENIZATION_MODE iNewMode
 * KLEL_NODE_TYPE         iToken
 * int                    iCount
 * int                    iFudge
 * const char             *pcPattern
 * KLEL_TOKEN_TRANSFORMER *pfTransformer
 *
 * To match a pattern, the following must be true:
 *   - The current tokenization mode must be equal to iMode
 *   - The iCount field must match the value returned by sscanf(1) when
 *     sscanf(1) is passed the pattern in pcPattern and input at the
 *     current offset.
 *
 * If a pattern matches, the following actions are taken:
 *   - The tokenization mode is switched to iNewMode.
 *   - The input is advanced by the length of the matched string, plus
 *     the fudge factor, plus the any skipped whitespace (as explained
 *     below).
 *   - The newly generated token is passed to the pfTransformer function
 *     if one is specified.
 *
 * The first pattern to match wins. In general, matches should be in
 * order from most-specific to least. An easy rule of thumb is to order
 * them in reverse order of their iCount member.
 *
 * The pcPattern member is where the magic happens. It is a format
 * string suitable for passing to sscanf(1). If the tokenizer should
 * skip whitespace before testing the pattern, the first character must
 * be a space character. Then, a mandatory %n conversion must appear.
 * This is used by the tokenizer to determine how much whitespace was
 * skipped. Finally, one to six conversions may be specified, with any
 * number of intervening literal characters. The fudge factor should be
 * equal to the number of literal characters in the pattern.
 *
 * When a pattern is matched, the one-to-six conversions are
 * concatenated and passed to the transformer function (if specified).
 *
 ***********************************************************************
 */
KLEL_TOKEN_PATTERN gasTokenPatterns[] =
{
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_REAL,          6, 0, " %n%255[0-9]%1[.]%255[0-9]%1[e]%1[+-]%255[0-9]", KlelRealToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_REAL,          5, 0, " %n%255[0-9]%1[.]%255[0-9]%1[e]%255[0-9]", KlelRealToToken},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      3, 1, "%n\\%1[x]%1[0-9A-Fa-f]%1[0-9A-Fa-f]", KlelCharToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_INTEGER,       3, 0, " %n%1[0]%1[x]%255[0-9A-Fa-f]", KlelIntegerToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_INTEGER,       3, 0, " %n%1[0]%1[o]%255[0-7]", KlelIntegerToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_REAL,          3, 0, " %n%255[0-9]%1[.]%255[0-9]", KlelRealToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_AND_AND,       2, 0, " %n%1[&]%1[&]"},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_INTERP, KLEL_NODE_DISCARD,       2, 0, "%n%1[%]%1[{]"},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_QUOTED, KLEL_NODE_DISCARD,       2, 0, "%n%1[%]%1[(]"},
  {KLEL_TOKENIZE_INTERP, KLEL_TOKENIZE_INTERP, KLEL_NODE_INTERP,        2, 0, " %n%1[A-Za-z_]%255[A-Za-z0-9_]", KlelDesignatorToToken},
  {KLEL_TOKENIZE_QUOTED, KLEL_TOKENIZE_QUOTED, KLEL_NODE_QUOTED_INTERP, 2, 0, " %n%1[A-Za-z_]%255[A-Za-z0-9_]", KlelDesignatorToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_DESIGNATOR,    2, 0, " %n%1[A-Za-z_]%255[A-Za-z0-9_]", KlelDesignatorToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_EQ_EQ,         2, 0, " %n%1[=]%1[=]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_GT_GT,         2, 0, " %n%1[>]%1[>]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_GTE,           2, 0, " %n%1[>]%1[=]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_LIKE,          2, 0, " %n%1[=]%1[~]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_LT_LT,         2, 0, " %n%1[<]%1[<]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_LTE,           2, 0, " %n%1[<]%1[=]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_NE,            2, 0, " %n%1[!]%1[=]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_PIPE_PIPE,     2, 0, " %n%1[|]%1[|]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_UNLIKE,        2, 0, " %n%1[!]%1[~]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_AND,           1, 0, " %n%1[&]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_BANG,          1, 0, " %n%1[!]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_CARET,         1, 0, " %n%1[ ^]"}, /* The space is necessary to avoid an invalid class. */
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_CLOSE_PAREN,   1, 0, " %n%1[)]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_CLOSE_SQUARE,  1, 0, " %n%1[]]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_COLON,         1, 0, " %n%1[:]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_COMMA,         1, 0, " %n%1[,]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_DESIGNATOR,    1, 0, " %n%1[A-Za-z_]", KlelDesignatorToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_DOT,           1, 0, " %n%1[.]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_EQ,            1, 0, " %n%1[=]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_GT,            1, 0, " %n%1[>]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_INTEGER,       1, 0, " %n%255[0-9]", KlelIntegerToToken},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_LT,            1, 0, " %n%1[<]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_MINUS,         1, 0, " %n%1[-]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_OPEN_PAREN,    1, 0, " %n%1[(]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_OPEN_SQUARE,   1, 0, " %n%1[[]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_PERCENT,       1, 0, " %n%1[%%]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_PIPE,          1, 0, " %n%1[|]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_PLUS,          1, 0, " %n%1[+]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_QUESTION,      1, 0, " %n%1[?]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_STRING, KLEL_NODE_QUOTE,         1, 0, " %n%1[\"]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_SLASH,         1, 0, " %n%1[/]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_STAR,          1, 0, " %n%1[*]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_TILDE,         1, 0, " %n%1[~]"},
  {KLEL_TOKENIZE_INTERP, KLEL_TOKENIZE_NORMAL, KLEL_NODE_QUOTE,         1, 0, "%n%1[\"]"},
  {KLEL_TOKENIZE_QUOTED, KLEL_TOKENIZE_NORMAL, KLEL_NODE_QUOTE,         1, 0, "%n%1[\"]"},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_NORMAL, KLEL_NODE_QUOTE,         1, 0, "%n%1[\"]"},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      1, 1, "%n\\%1[%%]", KlelCharToToken},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      1, 1, "%n\\%1[\\]", KlelCharToToken},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      1, 1, "%n\\%1[\"]", KlelCharToToken},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      1, 1, "%n\\%1[n]",  KlelCharToToken},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      1, 1, "%n\\%1[r]",  KlelCharToToken},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_FRAGMENT,      1, 0, "%n%255[^%\"\\]", KlelFragmentToToken},
  {KLEL_TOKENIZE_INTERP, KLEL_TOKENIZE_INTERP, KLEL_NODE_INTERP,        1, 0, " %n%1[A-Za-z_]", KlelDesignatorToToken},
  {KLEL_TOKENIZE_QUOTED, KLEL_TOKENIZE_QUOTED, KLEL_NODE_QUOTED_INTERP, 1, 0, " %n%1[A-Za-z_]", KlelDesignatorToToken},
  {KLEL_TOKENIZE_INTERP, KLEL_TOKENIZE_STRING, KLEL_NODE_DISCARD,       1, 0, " %n%1[}]"},
  {KLEL_TOKENIZE_QUOTED, KLEL_TOKENIZE_STRING, KLEL_NODE_DISCARD,       1, 0, " %n%1[)]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_EOF,           0, 0, " %n%255[a]"},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_EOF,           0, 0, " %n%255[a]"},
  {KLEL_TOKENIZE_NORMAL, KLEL_TOKENIZE_NORMAL, KLEL_NODE_EOF,         EOF, 0, " %n%255[a]"},
  {KLEL_TOKENIZE_STRING, KLEL_TOKENIZE_STRING, KLEL_NODE_EOF,         EOF, 0, " %n%255[a]"}
};


/*-
 ***********************************************************************
 *
 * KlelNextToken
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelNextToken(KLEL_CONTEXT *psContext)
{
  KLEL_NODE *psToken = KlelInnerNextToken(psContext);

  if (psToken != NULL && psToken->iType == KLEL_NODE_DISCARD)
  {
    KlelFreeNode(psToken);
    psToken = KlelInnerNextToken(psContext);
  }

  return psToken;
}


/*-
 ***********************************************************************
 *
 * KlelInnerNextToken
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelInnerNextToken(KLEL_CONTEXT *psContext)
{
  char               acBuf1[KLEL_MAX_NAME]           = {0};
  char               acBuf2[KLEL_MAX_NAME]           = {0};
  char               acBuf3[KLEL_MAX_NAME]           = {0};
  char               acBuf4[KLEL_MAX_NAME]           = {0};
  char               acBuf5[KLEL_MAX_NAME]           = {0};
  char               acBuf6[KLEL_MAX_NAME]           = {0};
  char               acBuffer[KLEL_MAX_NAME * 6 + 1] = {0};
  int                iCount                          = 0;
  int                iOff                            = 0;
  size_t             szi                             = 0;
  KLEL_TOKEN_PATTERN *psMatch                        = NULL;
  KLEL_NODE          *psNode                         = calloc(1, sizeof(KLEL_NODE));

  KLEL_ASSERT(psContext          != NULL);
  KLEL_ASSERT(psContext->pcInput != NULL);

  for (szi = 0; szi < sizeof(gasTokenPatterns) / sizeof(KLEL_TOKEN_PATTERN) && psNode != NULL && psNode->iType == KLEL_NODE_ERROR; szi++)
  {
    memset(acBuf1, 0, KLEL_MAX_NAME);
    memset(acBuf2, 0, KLEL_MAX_NAME);
    memset(acBuf3, 0, KLEL_MAX_NAME);
    memset(acBuf4, 0, KLEL_MAX_NAME);
    memset(acBuf5, 0, KLEL_MAX_NAME);
    memset(acBuf6, 0, KLEL_MAX_NAME);
    psMatch = &(gasTokenPatterns[szi]);

    if (psContext->iTokenizationMode == psMatch->iMode)
    {
      switch (psMatch->iCount)
      {
        case 0:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1);                                              break;
        case 1:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1);                                              break;
        case 2:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1, &acBuf2);                                     break;
        case 3:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1, &acBuf2, &acBuf3);                            break;
        case 4:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1, &acBuf2, &acBuf3, &acBuf4);                   break;
        case 5:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1, &acBuf2, &acBuf3, &acBuf4, &acBuf5);          break;
        case 6:   iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1, &acBuf2, &acBuf3, &acBuf4, &acBuf5, &acBuf6); break;
        case EOF: iCount = sscanf(psContext->pcInput, psMatch->pcPattern, &iOff, &acBuf1);                                              break;
        default:  KLEL_ASSERT(0);                                                                                                       break;
      }

      if (iCount == psMatch->iCount)
      {
        snprintf(acBuffer, KLEL_MAX_NAME * 6, "%s%s%s%s%s%s", acBuf1, acBuf2, acBuf3, acBuf4, acBuf5, acBuf6);
        psNode->iType = psMatch->iType;

        if (psMatch->pfTransformer != NULL)
        {
          psNode = psMatch->pfTransformer(psContext, psNode, acBuffer);
        }

        if (psNode != NULL && psNode->iType != KLEL_NODE_EOF)
        {
          psContext->iTokenizationMode = psMatch->iNewMode;
          psContext->pcInput = psContext->pcInput + iOff + strlen(acBuffer) + psMatch->iFudge;
        }
      }
    }
  }

  if (psNode == NULL || psNode->iType == KLEL_NODE_ERROR)
  {
    KlelFreeNode(psNode);
    psNode = NULL;
  }

  return psNode;
}


/*-
 ***********************************************************************
 *
 * KlelPeekToken
 *
 ***********************************************************************
 */
KLEL_NODE_TYPE
KlelPeekToken(KLEL_CONTEXT *psContext, unsigned int uiCount)
{
  const char             *pcCurrentInput          = psContext->pcInput;
  KLEL_TOKENIZATION_MODE iCurrentTokenizationMode = psContext->iTokenizationMode;
  KLEL_NODE              *psToken                 = NULL;
  KLEL_NODE_TYPE         iType                    = KLEL_NODE_ERROR;

  while (uiCount > 0)
  {
    psToken = KlelNextToken(psContext);
    if (psToken == NULL)
    {
      return KLEL_NODE_ERROR;
    }
    iType = psToken->iType;
    free(psToken);
    uiCount--;
  }

  psContext->pcInput           = pcCurrentInput;
  psContext->iTokenizationMode = iCurrentTokenizationMode;

  return iType;
}


/*-
 ***********************************************************************
 *
 * KlelCharToToken
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCharToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer)
{
  char acHexBuffer[KLEL_MAX_NAME + 1] = {0};
  int  iInt                           = 0;

  KLEL_ASSERT(psNode           != NULL);
  KLEL_ASSERT(psNode->iType    == KLEL_NODE_FRAGMENT);
  KLEL_ASSERT(pcBuffer         != NULL);
  KLEL_ASSERT(strlen(pcBuffer) == 1 || strlen(pcBuffer) == 3);

  if (psNode != NULL)
  {
    switch (pcBuffer[0])
    {
      case '%':
        psNode->acFragment[0] = '%';
        break;

      case '\\':
        psNode->acFragment[0] = '\\';
        break;

      case '"':
        psNode->acFragment[0] = '"';
        break;

      case 'r':
        psNode->acFragment[0] = '\r';
        break;

      case 'n':
        psNode->acFragment[0] = '\n';
        break;

      case 'x':
        snprintf(acHexBuffer, KLEL_MAX_NAME, "0x%c%c", pcBuffer[1], pcBuffer[2]);
        sscanf(acHexBuffer, "%x", &iInt);
        psNode->acFragment[0] = (char)iInt;
        break;
    }
  }

  psNode->szLength = 1;
  return psNode;
}


/*-
 ***********************************************************************
 *
 * KlelDesignatorToToken
 * In addition to transforming designators into tokens, this function
 * also parses out keywords and transforms them into their individual
 * tokens.
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelDesignatorToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer)
{
  size_t szi                       = 0;
  const char *apcKeywords[]        = {"eval",         "fail",         "if",         "in",         "let",         "pass",         "then"};
  KLEL_NODE_TYPE aiKeywordTokens[] = {KLEL_NODE_EVAL, KLEL_NODE_FAIL, KLEL_NODE_IF, KLEL_NODE_IN, KLEL_NODE_LET, KLEL_NODE_PASS, KLEL_NODE_THEN};

  KLEL_ASSERT(psNode         != NULL);
  KLEL_ASSERT(pcBuffer       != NULL);
  KLEL_ASSERT(sizeof(apcKeywords) / sizeof(const char *) == sizeof(aiKeywordTokens) / sizeof(KLEL_NODE_TYPE));
  KLEL_ASSERT(psNode->iType == KLEL_NODE_DESIGNATOR || psNode->iType == KLEL_NODE_INTERP
                                                    || psNode->iType == KLEL_NODE_QUOTED_INTERP);

  for (szi = 0; szi < sizeof(apcKeywords) / sizeof(const char *); szi++)
  {
    if (strcmp(pcBuffer, apcKeywords[szi]) == 0)
    {
      psNode->iType = aiKeywordTokens[szi];
    }
  }

  if (psNode->iType == KLEL_NODE_DESIGNATOR || psNode->iType == KLEL_NODE_INTERP || psNode->iType == KLEL_NODE_QUOTED_INTERP)
  {
    snprintf(psNode->acFragment, KLEL_MAX_NAME, "%s", pcBuffer);
  }

  return psNode;
}


/*-
 ***********************************************************************
 *
 * KlelFragmentToToken
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelFragmentToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer)
{
  size_t szLength = strlen(pcBuffer);

  KLEL_ASSERT(psNode           != NULL);
  KLEL_ASSERT(psNode->iType    == KLEL_NODE_FRAGMENT);
  KLEL_ASSERT(pcBuffer         != NULL);
  KLEL_ASSERT(strlen(pcBuffer) != 0);

  if (psNode != NULL)
  {
    memcpy(psNode->acFragment, pcBuffer, szLength >= KLEL_MAX_NAME ? KLEL_MAX_NAME : szLength);
    psNode->szLength = szLength;
  }

  return psNode;
}


/*-
 ***********************************************************************
 *
 * KlelIntegerToToken
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelIntegerToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer)
{
  char *pcEnd  = NULL;
  int  iOldErr = errno;

  KLEL_ASSERT(psNode           != NULL);
  KLEL_ASSERT(psNode->iType    == KLEL_NODE_INTEGER);
  KLEL_ASSERT(pcBuffer         != NULL);
  KLEL_ASSERT(strlen(pcBuffer) != 0);

  errno = 0;

  if (pcBuffer[0] == '0' && pcBuffer[1] == 'x')
  {
    psNode->llInteger = strtol(pcBuffer + 2, &pcEnd, 16);
  }
  else if (pcBuffer[0] == '0' && pcBuffer[1] == 'o')
  {
    psNode->llInteger = strtol(pcBuffer + 2, &pcEnd, 8);
  }
  else
  {
    psNode->llInteger = strtol(pcBuffer, &pcEnd, 10);
  }

  if (errno != 0)
  {
    KlelReportError(psContext, "invalid integer %s", pcBuffer, NULL);
    free(psNode);
    psNode = NULL;
  }

  errno = iOldErr;
  return psNode;
}


/*-
 ***********************************************************************
 *
 * KlelIntegerToToken
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelRealToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer)
{
  int  iOldErr = errno;
  char *pcEnd = NULL;

  KLEL_ASSERT(psNode           != NULL);
  KLEL_ASSERT(psNode->iType    == KLEL_NODE_REAL);
  KLEL_ASSERT(pcBuffer         != NULL);
  KLEL_ASSERT(strlen(pcBuffer) != 0);

  errno         = 0;
  psNode->dReal = strtod(pcBuffer, &pcEnd);

  if (errno != 0)
  {
    KlelReportError(psContext, "invalid real constant near %s", pcBuffer, NULL);
    free(psNode);
    psNode = NULL;
  }

  errno = iOldErr;
  return psNode;
}
