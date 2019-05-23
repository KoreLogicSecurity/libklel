/*-
 ***********************************************************************
 *
 * $Id: parser.h,v 1.11 2012/04/26 00:53:31 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#ifndef _PARSER_H_INCLUDED
#define _PARSER_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Utility Function Prototypes
 *
 ***********************************************************************
 */
KLEL_NODE *KlelCreateNode(const char *pcName, const char *pcInput, const char *pcRemainder, size_t szCount, ...);
KLEL_NODE *KlelParse(const char *pcInput, KLEL_CONTEXT *psContext);
void KlelPopError(KLEL_CONTEXT *psContext);
const char *KlelSpace(const char *pcInput);
KLEL_NODE *KlelTerminal(const char *pcRegex, const char *pcName, const char *pcInput);

/*-
 ***********************************************************************
 *
 * Matching Function Prototypes
 *
 ***********************************************************************
 */
KLEL_NODE *KlelAddOperator(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelCharacter(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelConditional(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelExpression(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelCall(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelDesignator(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelExprList(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelFactor(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelGuardedCommand(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelMulOperator(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelNumber(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelQuotedDesignator(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelRelation(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelSimpleExpression(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelString(const char *pcInput, KLEL_CONTEXT *psContext);
KLEL_NODE *KlelTerm(const char *pcInput, KLEL_CONTEXT *psContext);

#endif /* !_PARSER_H_INCLUDED */
