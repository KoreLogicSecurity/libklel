/*-
 ***********************************************************************
 *
 * $Id: tokenizer.h,v 1.8 2019/07/31 15:59:27 klm Exp $
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
#ifndef _TOKENIZER_H_INCLUDED
#define _TOKENIZER_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Tokenization Modes
 *
 ***********************************************************************
 */
typedef enum
{
  KLEL_TOKENIZE_NORMAL,
  KLEL_TOKENIZE_STRING,
  KLEL_TOKENIZE_QUOTED,
  KLEL_TOKENIZE_INTERP
} KLEL_TOKENIZATION_MODE;

/*-
 ***********************************************************************
 *
 * KlelNextToken and KlelPeekToken
 *
 ***********************************************************************
 */
KLEL_NODE      *KlelNextToken(KLEL_CONTEXT *psContext);
KLEL_NODE      *KlelInnerNextToken(KLEL_CONTEXT *psContext);
KLEL_NODE_TYPE KlelPeekToken(KLEL_CONTEXT *psContext, unsigned int uiCount);

/*-
 ***********************************************************************
 *
 * Token Transformation Function Prototypes
 *
 ***********************************************************************
 */
typedef KLEL_NODE *(*KLEL_TOKEN_TRANSFORMER)(KLEL_CONTEXT *, KLEL_NODE *, const char *);

KLEL_NODE *KlelCharToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer);
KLEL_NODE *KlelDesignatorToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer);
KLEL_NODE *KlelFragmentToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer);
KLEL_NODE *KlelIntegerToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer);
KLEL_NODE *KlelRealToToken(KLEL_CONTEXT *psContext, KLEL_NODE *psNode, const char *pcBuffer);

/*-
 ***********************************************************************
 *
 * Token Matching Pattern Structure
 *
 ***********************************************************************
 */
typedef struct _KLEL_TOKEN_PATTERN
{
  KLEL_TOKENIZATION_MODE iMode;
  KLEL_TOKENIZATION_MODE iNewMode;
  KLEL_NODE_TYPE         iType;
  int                    iCount;
  int                    iFudge;
  const char             *pcPattern;
  KLEL_TOKEN_TRANSFORMER pfTransformer;
} KLEL_TOKEN_PATTERN;

#endif /* ! _TOKENIZER_H_INCLUDED */
