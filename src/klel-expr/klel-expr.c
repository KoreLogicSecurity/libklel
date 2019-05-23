/*-
 ***********************************************************************
 *
 * $Id: klel-expr.c,v 1.36 2012/04/25 18:21:19 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <klel.h>
#include "klel-expr.h"

/*-
 ***********************************************************************
 *
 * Main
 *
 ***********************************************************************
 */
int
main(int argc, char **argv)
{
  int          i          = 0;
  const char   *pcExpr    = NULL;
  char         *pcError   = NULL;
  char         *pcName    = NULL;
  KLEL_COMMAND *psCommand = NULL;
  KLEL_CONTEXT *psContext = NULL;
  KLEL_NODE    *psTree    = NULL;
  KLEL_VALUE   *psResult  = NULL;
  size_t       szLength   = 0;
  char         *pcString  = NULL;
  char         *pcValue   = NULL;

  KlelGetTypeOfVar    = KlelGetTypeOfStdVar;
  KlelGetValueOfVar   = KlelGetValueOfStdVar;
  KlelGetFuncDesc     = KlelGetStdLibFuncDesc;

  if (argc < 2 || argc > 3)
  {
    fprintf(stderr, "\nUsage: %s [-p] <expression>\n\n", PROGRAM_NAME);
    return 1; /* XER_Usage */
  }

  if (argc == 3 && strcmp(argv[1], "-p") != 0)
  {
    fprintf(stderr, "\nUsage: %s [-p] <expression>\n\n", PROGRAM_NAME);
    return 1; /* XER_Usage */
  }

  psContext = KlelCreateContext(NULL);
  if (psContext == NULL)
  {
    fprintf(stderr, "out of memory\n");
    return 2; /* XER_Setup */
  }

  pcExpr = (argc == 3) ? argv[2] : argv[1];

  psTree = KlelCompile(pcExpr, 0, psContext);
  if (psTree == NULL)
  {
    pcError = KlelGetFirstError(psContext);
    while (pcError != NULL)
    {
      fprintf(stderr, "error: %s\n", pcError);
      pcError = KlelGetNextError(psContext);
    }

    KlelFreeContext(psContext);
    return 3; /* XER_Compile */
  }

  psResult = KlelExecute(psTree, psContext);
  if (psResult == NULL)
  {
    pcError = KlelGetFirstError(psContext);
    while (pcError != NULL)
    {
      fprintf(stderr, "error: %s\n", pcError);
      pcError = KlelGetNextError(psContext);
    }

    KlelFreeContext(psContext);
    KlelFreeNode(&psTree);
    return 4; /* XER_Execute */
  }

  if (KlelIsGuardedCommand(psTree) && psResult->bBoolean)
  {
    psCommand = KlelGetCommand(psTree, psContext);
    if (psCommand != NULL)
    {
      if (argc == 3 && strcmp(argv[1], "-p") == 0)
      {
        pcString = KlelExpressionToString(psTree, KLEL_EXPRESSION_PLUS_EVERYTHING);
        if (pcString != NULL)
        {
          printf("%s\n", pcString);
          printf("=====\n");
          free(pcString);
        }
      }
      printf("%s ", psCommand->pcInterpreter);
      for (i = 0; i < KLEL_MAX_FUNC_ARGS && psCommand->ppcArgumentVector[i] != NULL; i++)
      {
        printf("%s%s", (i == 0) ? "" : " ", psCommand->ppcArgumentVector[i]);
      }
      printf("\n");

      KlelFreeCommand(psCommand);
      KlelFreeResult(psResult);
      KlelFreeContext(psContext);
      KlelFreeNode(&psTree);
      return 0; /* XER_OK */
    }

    pcError = KlelGetFirstError(psContext);
    while (pcError != NULL)
    {
      fprintf(stderr, "error: %s\n", pcError);
      pcError = KlelGetNextError(psContext);
    }
    KlelFreeCommand(psCommand);
    KlelFreeResult(psResult);
    KlelFreeContext(psContext);
    KlelFreeNode(&psTree);
    return 5; /* XER_GuardedCommand */
  }

  if (argc == 3 && strcmp(argv[1], "-p") == 0)
  {
    pcString = KlelExpressionToString(psTree, KLEL_EXPRESSION_PLUS_EVERYTHING);
    if (pcString != NULL)
    {
      printf("%s\n", pcString);
      printf("=====\n");
      free(pcString);
    }
  }

  pcName  = KlelGetName(psTree);
  pcValue = KlelValueToString(psResult, &szLength);

  if (pcName == NULL || pcValue == NULL)
  {
    fprintf(stderr, "out of memory\n");
    exit(3);
  }

  switch (psResult->iType)
  {
    case KLEL_EXPR_BOOLEAN:
      printf("%s : bool : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_FLOAT:
      printf("%s : float : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_FUNCTION_BOOLEAN:
      printf("%s : func -> bool : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_FUNCTION_FLOAT:
      printf("%s : func -> float : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_FUNCTION_INTEGER:
      printf("%s : func -> int : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_FUNCTION_STRING:
      printf("%s : func -> string : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_INTEGER:
      printf("%s : int : %s\n", pcName, pcValue);
      break;

    case KLEL_EXPR_STRING:
      printf("%s : string : %s\n", pcName, pcValue);
      break;

    default:
      KLEL_ASSERT(0);
      break;
  }

  free(pcName);
  free(pcValue);
  KlelFreeResult(psResult);
  KlelFreeContext(psContext);
  KlelFreeNode(&psTree);

  return 0; /* XER_OK */
}
