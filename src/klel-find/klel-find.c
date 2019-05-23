/*-
 ***********************************************************************
 *
 * $Id: klel-find.c,v 1.49 2012/04/25 18:21:19 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#include "config.h"

#define _BSD_SOURCE       /* For DT_DIR             */
#define _XOPEN_SOURCE 500 /* For S_ISVTX and nftw() */
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <klel.h>
#include "klel-find.h"

/*-
 ***********************************************************************
 *
 * The compiled expression, stored globally since it's compiled only
 * once and used many times.
 *
 ***********************************************************************
 */
KLEL_NODE *gpsExpression = NULL;

/*-
 ***********************************************************************
 *
 * The context, stored globally since there's only one.
 *
 ***********************************************************************
 */
KLEL_CONTEXT *gpsContext = NULL;

/*-
 ***********************************************************************
 *
 * KlelFindDefined
 * This returns true or false if the named variable is defined. It is
 * provided as an example of how to handle undefined variables.
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelFindDefined(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  size_t            szi         = 0;
  KLEL_FIND_ITEM    *psItem     = (KLEL_FIND_ITEM *)(psContext->pvData);
  const struct stat *psStat     = psItem->psStat;
  const char        *apcNames[] =
  {
    "basename",
    "defined",
    "path",
    "mode",
    "uid",
    "gid",
    "owner",
    "group",
    "size",
    "atime",
    "mtime",
    "ctime",
    "directory",
    "device",
    "block_device",
    "char_device",
    "fifo",
    "inode",
    "suid",
    "sgid",
    "sticky",
    "u_r",
    "u_w",
    "u_x",
    "g_r",
    "g_w",
    "g_x",
    "o_r",
    "o_w",
    "o_x"
  };

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_STRING);

  /* Check if the variable is always defined. */
  for (szi = 0; szi < sizeof(apcNames) / sizeof(const char *); szi++)
  {
    if (strcmp(apcNames[szi], asArgs[0]->acString) == 0)
    {
      return KlelCreateBoolean(1);
    }
  }

  /* Otherwise check the special case variables. */
  if (strcmp(asArgs[0]->acString, "rdev") == 0)
  {
    if (S_ISCHR(psStat->st_mode) || S_ISBLK(psStat->st_mode))
    {
      return KlelCreateBoolean(1);
    }
  }

  return KlelCreateBoolean(0);
}

/*-
 ***********************************************************************
 *
 * KlelFindBasename
 * This computes the basename of a path. It is provided as an example
 * of how to export a function into the Klel environment.
 *
 ***********************************************************************
 */
KLEL_VALUE *
KlelFindBasename(KLEL_VALUE **asArgs, KLEL_CONTEXT *psContext)
{
  char acBuffer[FILENAME_MAX + 1];
  char *pcTemp = NULL;

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_EXPR_STRING);

  strncpy(acBuffer, asArgs[0]->acString, FILENAME_MAX);
  pcTemp = basename(acBuffer);
  return KlelCreateString(strlen(pcTemp), pcTemp);
}

/*-
 ***********************************************************************
 *
 * GetFuncDesc
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE *
GetFuncDesc(const char *pcName, KLEL_CONTEXT *psContext)
{
  size_t         szi = 0;
  KLEL_EXPR_TYPE *aiArguments = NULL;
  KLEL_FUNC_DESC asFuncs[] = {
    {"defined",  {KLEL_EXPR_STRING}},
    {"basename", {KLEL_EXPR_STRING}}
  };

  KLEL_ASSERT(pcName != NULL);

  for (szi = 0; szi < (sizeof(asFuncs) / sizeof(KLEL_FUNC_DESC)); szi++)
  {
    if (strcmp(asFuncs[szi].pcName, pcName) == 0)
    {
      aiArguments = calloc(1, sizeof(KLEL_EXPR_TYPE) * KLEL_MAX_FUNC_ARGS);
      if (aiArguments != NULL)
      {
        memcpy(aiArguments, asFuncs[szi].aiArguments, sizeof(KLEL_EXPR_TYPE) * KLEL_MAX_FUNC_ARGS);
        return aiArguments;
      }
      else
      {
        return NULL;
      }
    }
  }

  return KlelGetStdLibFuncDesc(pcName, psContext); /* Chain to the standard library. */
}


/*-
 ***********************************************************************
 *
 * GetTypeOfVar
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
GetTypeOfVar(const char *pcName, KLEL_CONTEXT *psContext)
{
  size_t         szi       = 0;
  KLEL_TYPE_SPEC asTypes[] =
  {
    {"basename",      KLEL_EXPR_FUNCTION_STRING},
    {"defined",       KLEL_EXPR_FUNCTION_BOOLEAN},
    {"path",          KLEL_EXPR_STRING},
    {"mode",          KLEL_EXPR_INTEGER},
    {"uid",           KLEL_EXPR_INTEGER},
    {"gid",           KLEL_EXPR_INTEGER},
    {"owner",         KLEL_EXPR_STRING},
    {"group",         KLEL_EXPR_STRING},
    {"size",          KLEL_EXPR_INTEGER},
    {"atime",         KLEL_EXPR_INTEGER},
    {"mtime",         KLEL_EXPR_INTEGER},
    {"ctime",         KLEL_EXPR_INTEGER},
    {"directory",     KLEL_EXPR_BOOLEAN},
    {"device",        KLEL_EXPR_BOOLEAN},
    {"block_device",  KLEL_EXPR_BOOLEAN},
    {"char_device",   KLEL_EXPR_BOOLEAN},
    {"fifo",          KLEL_EXPR_BOOLEAN},
    {"inode",         KLEL_EXPR_INTEGER},
    {"rdev",          KLEL_EXPR_INTEGER},
    {"suid",          KLEL_EXPR_BOOLEAN},
    {"sgid",          KLEL_EXPR_BOOLEAN},
    {"sticky",        KLEL_EXPR_BOOLEAN},
    {"u_r",           KLEL_EXPR_BOOLEAN},
    {"u_w",           KLEL_EXPR_BOOLEAN},
    {"u_x",           KLEL_EXPR_BOOLEAN},
    {"g_r",           KLEL_EXPR_BOOLEAN},
    {"g_w",           KLEL_EXPR_BOOLEAN},
    {"g_x",           KLEL_EXPR_BOOLEAN},
    {"o_r",           KLEL_EXPR_BOOLEAN},
    {"o_w",           KLEL_EXPR_BOOLEAN},
    {"o_x",           KLEL_EXPR_BOOLEAN}
  };

  for (szi = 0; szi < (sizeof(asTypes) / sizeof(KLEL_TYPE_SPEC)); szi++)
  {
    if (strcmp(asTypes[szi].pcName, pcName) == 0)
    {
      return asTypes[szi].iType;
    }
  }

  return KlelGetTypeOfStdVar(pcName, psContext); /* Chain to the standard library. */
}


/*-
 ***********************************************************************
 *
 * GetValueOfVar
 *
 ***********************************************************************
 */
KLEL_VALUE *
GetValueOfVar(const char *pcName, KLEL_CONTEXT *psContext)
{
  char               acBuffer[FILENAME_MAX + 1];
  struct group      *psGroup  = NULL;
  struct passwd     *psPasswd = NULL;
  KLEL_FIND_ITEM    *psItem   = (KLEL_FIND_ITEM *)(psContext->pvData);
  const char        *pcPath   = psItem->pcName;
  const struct stat *psStat   = psItem->psStat;

  KLEL_ASSERT(pcName != NULL);
  KLEL_ASSERT(pcPath != NULL);

  if (strcmp(pcName, "basename") == 0)
  {
    return KlelCreateStringFunc("basename", KlelFindBasename);
  }
  else if (strcmp(pcName, "defined") == 0)
  {
    return KlelCreateBooleanFunc("defined", KlelFindDefined);
  }
  else if (strcmp(pcName, "path") == 0)
  {
    return KlelCreateString(strlen(pcPath), pcPath);
  }
  else if (strcmp(pcName, "mode") == 0)
  {
    return KlelCreateInteger(psStat->st_mode);
  }
  else if (strcmp(pcName, "uid") == 0)
  {
    return KlelCreateInteger(psStat->st_uid);
  }
  else if (strcmp(pcName, "gid") == 0)
  {
    return KlelCreateInteger(psStat->st_gid);
  }
  else if (strcmp(pcName, "size") == 0)
  {
    return KlelCreateInteger(psStat->st_size);
  }
  else if (strcmp(pcName, "atime") == 0)
  {
    return KlelCreateInteger(psStat->st_atime);
  }
  else if (strcmp(pcName, "mtime") == 0)
  {
    return KlelCreateInteger(psStat->st_mtime);
  }
  else if (strcmp(pcName, "ctime") == 0)
  {
    return KlelCreateInteger(psStat->st_ctime);
  }
  else if (strcmp(pcName, "directory") == 0)
  {
    return KlelCreateBoolean(S_ISDIR(psStat->st_mode));
  }
  else if (strcmp(pcName, "device") == 0)
  {
    return KlelCreateBoolean(S_ISCHR(psStat->st_mode) || S_ISBLK(psStat->st_mode));
  }
  else if (strcmp(pcName, "rdev") == 0)
  {
    if (S_ISCHR(psStat->st_mode) || S_ISBLK(psStat->st_mode))
    {
      return KlelCreateInteger(psStat->st_rdev);
    }

    return NULL;
  }
  else if (strcmp(pcName, "block_device") == 0)
  {
    return KlelCreateBoolean(S_ISBLK(psStat->st_mode));
  }
  else if (strcmp(pcName, "char_device") == 0)
  {
    return KlelCreateBoolean(S_ISCHR(psStat->st_mode));
  }
  else if (strcmp(pcName, "fifo") == 0)
  {
    return KlelCreateBoolean(S_ISFIFO(psStat->st_mode));
  }
  else if (strcmp(pcName, "inode") == 0)
  {
    return KlelCreateInteger(psStat->st_ino);
  }
  else if (strcmp(pcName, "suid") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_ISUID);
  }
  else if (strcmp(pcName, "sgid") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_ISGID);
  }
  else if (strcmp(pcName, "sticky") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_ISVTX);
  }
  else if (strcmp(pcName, "u_r") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IRUSR);
  }
  else if (strcmp(pcName, "u_w") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IWUSR);
  }
  else if (strcmp(pcName, "u_x") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IXUSR);
  }
  else if (strcmp(pcName, "g_r") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IRGRP);
  }
  else if (strcmp(pcName, "g_w") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IWGRP);
  }
  else if (strcmp(pcName, "g_x") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IXGRP);
  }
  else if (strcmp(pcName, "o_r") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IROTH);
  }
  else if (strcmp(pcName, "o_w") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IWOTH);
  }
  else if (strcmp(pcName, "o_x") == 0)
  {
    return KlelCreateBoolean(psStat->st_mode & S_IXOTH);
  }
  else if (strcmp(pcName, "owner") == 0)
  {
    psPasswd = getpwuid(psStat->st_uid);
    if (psPasswd == NULL)
    {
      memset(acBuffer, 0, sizeof(acBuffer));
      snprintf(acBuffer, sizeof(acBuffer) - 1, "%lu", (unsigned long)(psStat->st_uid));
      return KlelCreateString(strlen(acBuffer), acBuffer);
    }

    return KlelCreateString(strlen(psPasswd->pw_name), psPasswd->pw_name);
  }
  else if (strcmp(pcName, "group") == 0)
  {
    psGroup = getgrgid(psStat->st_gid);
    if (psPasswd == NULL)
    {
      memset(acBuffer, 0, sizeof(acBuffer));
      snprintf(acBuffer, sizeof(acBuffer) - 1, "%lu", (unsigned long)(psStat->st_uid));
      return KlelCreateString(strlen(acBuffer), acBuffer);
    }

    return KlelCreateString(strlen(psGroup->gr_name), psGroup->gr_name);
  }

  return KlelGetValueOfStdVar(pcName, psContext); /* Chain to the standard library. */
}


/*-
 ***********************************************************************
 *
 * ProcessDirectory
 *
 ***********************************************************************
 */
int
ProcessDirectory(const char *pcName, const struct stat *psStat, int iFlags, struct FTW *psFtwBuffer)
{
  KLEL_COMMAND   *psCommand = NULL;
  KLEL_CONTEXT   *psContext = NULL;
  pid_t          iPid       = 0;
  KLEL_VALUE     *psResult  = NULL;
  int            iStatus    = 0;
  KLEL_FIND_ITEM psItem     = {0};

  KLEL_ASSERT(pcName != NULL);

  psItem.pcName = pcName;
  psItem.psStat = psStat;
  psContext     = KlelCreateContext(&psItem);
  if (psContext == NULL)
  {
    fprintf(stderr, "out of memory");
    return 1;
  }

  psResult = KlelExecute(gpsExpression, psContext);
  if (psResult == NULL)
  {
    fprintf(stderr, "expression failed for %s: %s\n", pcName, KlelGetFirstError(psContext));
  }

  if (psResult != NULL && psResult->bBoolean)
  {
    if (KlelIsGuardedCommand(gpsExpression))
    {
      psCommand = KlelGetCommand(gpsExpression, psContext);
      if (psCommand != NULL)
      {
        if (strcmp(psCommand->pcInterpreter, "system") == 0)
        {
          iStatus = system(psCommand->pcProgram);
          if (iStatus == -1)
          {
            perror("unable to execute command");
          }
        }
        else if (strcmp(psCommand->pcInterpreter, "exec") == 0)
        {
          iPid = fork();
          if (iPid == 0)
          {
            if (execv(psCommand->pcProgram, psCommand->ppcArgumentVector) != 0)
            {
              perror("unable to execute command");
            }
            exit(0);
          }
          else
          {
            wait(&iStatus);
          }
        }

        if (!KlelIsSuccessReturnCode(gpsExpression, WEXITSTATUS(iStatus)))
        {
          fprintf(stderr, "guard matched for path %s, but command exited with status %d\n", pcName, WEXITSTATUS(iStatus));
        }

        KlelFreeCommand(psCommand);
      }
      else
      {
        fprintf(stderr, "guard matched for path %s, but command was invalid: %s\n", pcName, KlelGetFirstError(psContext));
      }
    }
    else
    {
      printf("%s\n", pcName);
    }

    KlelFreeResult(psResult);
  }

  KlelFreeContext(psContext);

  return 0;
}

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
  char          *pcError   = NULL;

  /* Set up our callbacks. */
  KlelGetTypeOfVar    = GetTypeOfVar;
  KlelGetValueOfVar   = GetValueOfVar;
  KlelGetFuncDesc     = GetFuncDesc;

  if (argc < 3)
  {
    fprintf(stderr, "\nUsage: %s <directory> <expression>\n\n", PROGRAM_NAME);
    return 1;
  }

  gpsContext = KlelCreateContext(NULL);
  if (gpsContext == NULL)
  {
    fprintf(stderr, "out of memory\n");
    return 1;
  }

  gpsExpression = KlelCompile(argv[2], 0, gpsContext);
  if (gpsExpression == NULL)
  {
    pcError = KlelGetFirstError(gpsContext);
    while (pcError != NULL)
    {
      fprintf(stderr, "error: %s\n", pcError);
      pcError = KlelGetNextError(gpsContext);
    }
    return 1;
  }

  if (KlelGetTypeOfExpression(gpsExpression, gpsContext) != KLEL_EXPR_BOOLEAN)
  {
    fprintf(stderr, "error: %s requires predicates to be of Boolean type\n", argv[0]);
    return 1;
  }

  nftw(argv[1], ProcessDirectory, 2, 0);

  KlelFreeNode(&gpsExpression);

  return 0;
}
