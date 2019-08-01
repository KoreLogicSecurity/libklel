/*-
 ***********************************************************************
 *
 * $Id: parser.h,v 1.24 2019/07/31 15:59:27 klm Exp $
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
#ifndef _PARSER_H_INCLUDED
#define _PARSER_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Parser Production Macros
 * These macros build parser function declarations. They greatly
 * simplify the memory management of the parser by automatically
 * maintaining reference counts on tokens and produced nodes.
 *
 * Parser productions written using these macros should follow a
 * "load-store" approach. Every acquisition of a token (via KLEL_EXPECT,
 * KLEL_INVOKE, or KLEL_CREATE_NODE) should be wrapped in a call to
 * KLEL_CAPTURE_PRODUCTION. KLEL_CAPTURE_PRODUCTION retains a reference
 * to the node that will be released when the parsing function exits.
 * The macros KLEL_ADD_TO_RETURNED_NODE, KLEL_ROTATE_PRODUCTION,
 * KLEL_APPEND_PRODUCTION, etc, retain a reference to the node. If a
 * node's reference count is zero at the end of the function, the node
 * is freed, otherwise it is kept. Using these macros consistently
 * removes the burden of memory management from the programmer, which
 * could get hairy in complex productions or when trying to account for
 * all possible error conditions.
 *
 * These macros also contain a goto label that is goneto whenever an
 * error is detected; this centralizes error management for the parsing
 * code (you should've seen it before).
 *
 * (On second thought, you shouldn't have. Yikes. Same as for the memory
 * management before. Painful.)
 *
 ***********************************************************************
 */
typedef struct _KLEL_PRODUCTION_TOKEN_MAP
{
  const char *pcName;
  KLEL_NODE  *psToken;
} KLEL_PRODUCTION_TOKEN_MAP;

#define KLEL_PUSH_ENVIRONMENT(context, name)                                                                                   \
  do                                                                                                                           \
  {                                                                                                                            \
    KLEL_CLOSURE sClosure = {0};                                                                                               \
    KLEL_CLOSURE *psNewClosures = realloc(psContext->psClosures, (psContext->iClosureCount + 1) * sizeof(KLEL_CLOSURE));       \
    if (psNewClosures == NULL)                                                                                                 \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    else                                                                                                                       \
    {                                                                                                                          \
      psContext->psClosures = psNewClosures;                                                                                   \
    }                                                                                                                          \
    strncpy(sClosure.acVariable, name, KLEL_MAX_NAME);                                                                         \
    sClosure.iIndex = (context)->iClosureCount;                                                                                \
    sClosure.psNext = (context)->psEnvironment;                                                                                \
    (context)->iClosureCount++;                                                                                                \
    (context)->psEnvironment = &sClosure;                                                                                      \
    bPopEnvironment = 1;                                                                                                       \

#define KLEL_POP_ENVIRONMENT(context)                                                                                          \
    memcpy(&((context)->psClosures[sClosure.iIndex]), &sClosure, sizeof(KLEL_CLOSURE));                                        \
    (context)->psEnvironment = (context)->psEnvironment->psNext;                                                               \
    bPopEnvironment = 0;                                                                                                       \
  } while (0)

#define KLEL_SET_RETURNED_NODE_ENVIRONMENT(index)                                                                              \
  do                                                                                                                           \
  {                                                                                                                            \
    psReturnedNode->iClosure = (index);                                                                                        \
  } while (0)

#define KLEL_PARSER_PRODUCTION(Name)                                                                                           \
  KLEL_NODE *                                                                                                                  \
  Name (KLEL_CONTEXT *psContext)                                                                                               \
  {                                                                                                                            \
    int          iSuppressErrors  = psContext->iSuppressErrors;                                                                \
    const char   *pcStartInput    = psContext->pcInput;                                                                        \
    int          bPopEnvironment  = 0;                                                                                         \
    KLEL_NODE    *psReturnedNode  = calloc(1, sizeof(KLEL_NODE));

#define KLEL_END_PARSER_PRODUCTION                                                                                             \
    do                                                                                                                         \
    {                                                                                                                          \
      size_t szi = 0;                                                                                                          \
      for (szi = 0; szi < sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP); szi++)                               \
      {                                                                                                                        \
        if (asProductionTokens[szi].psToken != NULL)                                                                           \
        {                                                                                                                      \
          KlelReleaseNode(asProductionTokens[szi].psToken);                                                                    \
        }                                                                                                                      \
      }                                                                                                                        \
      if (bPopEnvironment && psContext->psEnvironment != NULL)                                                                 \
      {                                                                                                                        \
        psContext->psEnvironment = psContext->psEnvironment->psNext;                                                           \
      }                                                                                                                        \
    } while (0);                                                                                                               \
    psContext->iSuppressErrors = iSuppressErrors;                                                                              \
    pcStartInput = pcStartInput; /* Avoid a compiler warning. */                                                               \
    pcStartInput = NULL;         /* Avoid a compiler warning. */                                                               \
    return psReturnedNode;                                                                                                     \
    _klel_parser_error_exit:                                                                                                   \
    do                                                                                                                         \
    {                                                                                                                          \
      size_t szi = 0;                                                                                                          \
      for (szi = 0; szi < sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP); szi++)                               \
      {                                                                                                                        \
        if (asProductionTokens[szi].psToken != NULL)                                                                           \
        {                                                                                                                      \
          KlelReleaseNode(asProductionTokens[szi].psToken);                                                                    \
        }                                                                                                                      \
      }                                                                                                                        \
    } while (0);                                                                                                               \
    if (bPopEnvironment && psContext->psEnvironment != NULL)                                                                   \
    {                                                                                                                          \
      psContext->psEnvironment = psContext->psEnvironment->psNext;                                                             \
    }                                                                                                                          \
    if (psReturnedNode != NULL)                                                                                                \
    {                                                                                                                          \
      KlelReleaseNode(psReturnedNode);                                                                                         \
    }                                                                                                                          \
    psContext->iSuppressErrors = iSuppressErrors;                                                                              \
    return NULL;                                                                                                               \
  }

#define KLEL_PRODUCTION_TOKENS                                                                                                 \
    KLEL_PRODUCTION_TOKEN_MAP asProductionTokens[] =                                                                           \
    {

#define KLEL_TOKEN_DEF(Name)                                                                                                   \
      {#Name , NULL},                                                                                                          \

#define KLEL_END_PRODUCTION_TOKENS                                                                                             \
      {NULL, NULL}                                                                                                             \
    };

#define KLEL_PEEK()  KlelPeekToken(psContext, 1)
#define KLEL_PEEK2() KlelPeekToken(psContext, 2)

#define KLEL_CAPTURE_PRODUCTION(Name, Expression)                                                                              \
  do                                                                                                                           \
  {                                                                                                                            \
    int       iI                     = 0;                                                                                      \
    int       bFound                 = 0;                                                                                      \
    KLEL_NODE *psEvaluatedExpression = (Expression);                                                                           \
    if (psEvaluatedExpression == NULL)                                                                                         \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    for (iI = 0; iI < sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP); iI++)                                    \
    {                                                                                                                          \
      if (asProductionTokens[iI].pcName != NULL && strcmp(asProductionTokens[iI].pcName, #Name) == 0)                          \
      {                                                                                                                        \
        if (asProductionTokens[iI].psToken != NULL)                                                                            \
        {                                                                                                                      \
          KlelReleaseNode(asProductionTokens[iI].psToken);                                                                     \
        }                                                                                                                      \
        asProductionTokens[iI].psToken = psEvaluatedExpression;                                                                \
        bFound = 1;                                                                                                            \
        break;                                                                                                                 \
      }                                                                                                                        \
    }                                                                                                                          \
    if (bFound == 0)                                                                                                           \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    KLEL_ASSERT(bFound != 0);                                                                                                  \
  } while (0)

#define KLEL_ADD_TO_RETURNED_NODE(Index, Name)                                                                                 \
  do                                                                                                                           \
  {                                                                                                                            \
    KLEL_NODE *psNode = KLEL_CAPTURED_PRODUCTION(Name);                                                                        \
    KLEL_ASSERT(Index < KLEL_MAX_CHILDREN);                                                                                    \
    if (psReturnedNode == NULL || psNode == NULL)                                                                              \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    psReturnedNode->apsChildren[Index] = psNode;                                                                               \
    KlelRetainNode(psNode);                                                                                                    \
  } while (0)

#define KLEL_SET_RETURNED_NODE_TYPE(Type)                                                                                      \
  do                                                                                                                           \
  {                                                                                                                            \
    if (psReturnedNode == NULL)                                                                                                \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    psReturnedNode->iType = (Type);                                                                                            \
  } while (0)

#define KLEL_APPEND_PRODUCTION(Node, Index, Expression)                                                                        \
  do                                                                                                                           \
  {                                                                                                                            \
    KLEL_NODE *psEvaluatedExpression = KLEL_CAPTURED_PRODUCTION(Expression);                                                   \
    KLEL_NODE *psNode                = KLEL_CAPTURED_PRODUCTION(Node);                                                         \
    KLEL_ASSERT(Index < KLEL_MAX_CHILDREN);                                                                                    \
    if (psEvaluatedExpression == NULL)                                                                                         \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    for (; psNode != NULL && psNode->apsChildren[Index] != NULL; psNode = psNode->apsChildren[Index])                          \
      ;                                                                                                                        \
    psNode->apsChildren[Index] = psEvaluatedExpression;                                                                        \
    KlelRetainNode(psNode->apsChildren[Index]);                                                                                \
   } while (0)

#define KLEL_INPUT_NEAR(input) (strlen(KlelStripSpace(input)) == 0 ? "end of input" : input)

#define KLEL_EXPECT(Type) ((KLEL_PEEK() == (Type)) ? KlelNextToken(psContext)                                                  \
                                                   : (psContext->iSuppressErrors ? NULL                                        \
                                                                                : KlelReportError(psContext,                   \
                                                                                    "expected %s near %s",                     \
                                                                                    apcKlelNodeToString[Type],                 \
                                                                                    KLEL_INPUT_NEAR(psContext->pcInput),       \
                                                                                    NULL),                                     \
                                                      NULL))

#define KLEL_REPORT_ERROR(msg)                                                                                                 \
  do                                                                                                                           \
  {                                                                                                                            \
    if (psContext->iSuppressErrors == 0)                                                                                       \
    {                                                                                                                          \
      KlelReportError(psContext, "%s near %s", msg, KLEL_INPUT_NEAR(pcStartInput), NULL);                                      \
    }                                                                                                                          \
    goto _klel_parser_error_exit;                                                                                              \
  } while (0)

#define KLEL_INVOKE(Production) (Production (psContext))

#define KLEL_TRY_CAPTURE_PRODUCTION(Name, Expression)                                                                          \
  do                                                                                                                           \
  {                                                                                                                            \
    int        iI                       = 0;                                                                                   \
    int        iCurrentTokenizationMode = psContext->iTokenizationMode;                                                        \
    const char *pcCurrentInput          = psContext->pcInput;                                                                  \
    KLEL_NODE  *psEvaluatedExpression   = NULL;                                                                                \
    psContext->iSuppressErrors++;                                                                                              \
    psEvaluatedExpression = (Expression);                                                                                      \
    if (psEvaluatedExpression != NULL)                                                                                         \
    {                                                                                                                          \
      for (iI = 0; iI < sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP); iI++)                                  \
      {                                                                                                                        \
        if (asProductionTokens[iI].pcName != NULL && strcmp(asProductionTokens[iI].pcName, #Name) == 0)                        \
        {                                                                                                                      \
          if (asProductionTokens[iI].psToken != NULL)                                                                          \
          {                                                                                                                    \
            KlelReleaseNode(asProductionTokens[iI].psToken);                                                                   \
          }                                                                                                                    \
          asProductionTokens[iI].psToken = psEvaluatedExpression;                                                              \
          break;                                                                                                               \
        }                                                                                                                      \
      }                                                                                                                        \
    }                                                                                                                          \
    psContext->pcInput           = pcCurrentInput;                                                                             \
    psContext->iTokenizationMode = iCurrentTokenizationMode;                                                                   \
    psContext->iSuppressErrors--;                                                                                              \
  } while (0)

#define KLEL_CAPTURED_PRODUCTION(Name)                                                                                         \
  KlelCapturedProduction(#Name, asProductionTokens, sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP))

#define KLEL_COPY_PRODUCTION(Name)                                                                                             \
  KlelCopyProduction(KlelCapturedProduction(#Name, asProductionTokens, sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP)))

#define KLEL_REPLACE_RETURNED_NODE(Name)                                                                                       \
  do                                                                                                                           \
  {                                                                                                                            \
    if (psReturnedNode != NULL)                                                                                                \
    {                                                                                                                          \
      KlelReleaseNode(psReturnedNode);                                                                                         \
    }                                                                                                                          \
    psReturnedNode = KLEL_CAPTURED_PRODUCTION(Name);                                                                           \
    if (psReturnedNode == NULL)                                                                                                \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    KlelRetainNode(psReturnedNode);                                                                                            \
  } while (0)

#define KLEL_ROTATE_PRODUCTION(Production, Index, Expression)                                                                  \
  do                                                                                                                           \
  {                                                                                                                            \
    int iI                       = 0;                                                                                          \
    int bFound                   = 0;                                                                                          \
    KLEL_NODE *psProductionToken = NULL;                                                                                       \
    KLEL_NODE *psNewToken        = KLEL_CAPTURED_PRODUCTION(Expression);                                                       \
    KLEL_ASSERT(Index < KLEL_MAX_CHILDREN);                                                                                    \
    if (psNewToken == NULL)                                                                                                    \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    for (iI = 0; iI < sizeof(asProductionTokens) / sizeof(KLEL_PRODUCTION_TOKEN_MAP); iI++)                                    \
    {                                                                                                                          \
      if (asProductionTokens[iI].pcName != NULL && strcmp(asProductionTokens[iI].pcName, #Production) == 0)                    \
      {                                                                                                                        \
        KLEL_ASSERT(asProductionTokens[iI].psToken != NULL);                                                                   \
        psProductionToken = asProductionTokens[iI].psToken;                                                                    \
        bFound = 1;                                                                                                            \
        break;                                                                                                                 \
      }                                                                                                                        \
    }                                                                                                                          \
    KLEL_ASSERT(bFound != 0);                                                                                                  \
    KLEL_ASSERT(psProductionToken != NULL);                                                                                    \
    if (bFound == 0)                                                                                                           \
    {                                                                                                                          \
      goto _klel_parser_error_exit;                                                                                            \
    }                                                                                                                          \
    psNewToken->iReferenceCount    = psProductionToken->iReferenceCount;                                                       \
    psNewToken->apsChildren[Index] = psProductionToken;                                                                        \
    asProductionTokens[iI].psToken = psNewToken;                                                                               \
    KlelRetainNode(psNewToken); /* Referenced from here and its original capture. */                                           \
  } while (0);

#define KLEL_CREATE_NODE(Type) KlelCreateNode(Type)

/*-
 ***********************************************************************
 *
 * The Node Type to String Map
 *
 ***********************************************************************
 */
extern char *apcKlelNodeToString[];

/*-
 ***********************************************************************
 *
 * Parser Function Prototypes
 *
 ***********************************************************************
 */
KLEL_NODE  *KlelCapturedProduction(const char *pcName, KLEL_PRODUCTION_TOKEN_MAP *psMap, size_t szMapSize);
KLEL_NODE  *KlelCopyProduction(KLEL_NODE *psNode);
int        KlelClosureForDesignator(KLEL_CONTEXT *psContext, const char *pcName);
KLEL_NODE  *KlelConditional(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelCreateNode(KLEL_NODE_TYPE iType);
KLEL_NODE  *KlelExpression(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelFactor(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelLet(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelGuardedCommand(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelPotentialConditional(KLEL_CONTEXT *psContext);
void       KlelReleaseNode(KLEL_NODE *psNode);
void       KlelRetainNode(KLEL_NODE *psNode);
KLEL_NODE  *KlelRoot(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelSimpleExpression(KLEL_CONTEXT *psContext);
KLEL_NODE  *KlelString(KLEL_CONTEXT *psContext);
const char *KlelStripSpace(const char *pcInput);
KLEL_NODE  *KlelTerm(KLEL_CONTEXT *psContext);

#endif /* ! _PARSER_H_INCLUDED */
