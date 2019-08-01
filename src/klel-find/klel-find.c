/*-
 ***********************************************************************
 *
 * $Id: klel-find.c,v 1.56 2019/07/31 15:59:27 klm Exp $
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
#include "config.h"

#define _DEFAULT_SOURCE   /* For DT_DIR             */
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
KlelFindDefined(KLEL_VALUE **asArgs, void *ignored)
{
  size_t            szi         = 0;
  KLEL_FIND_ITEM    *psItem     = (KLEL_FIND_ITEM *)(KlelGetPrivateData(gpsContext));
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
  KLEL_ASSERT(asArgs[0]->iType == KLEL_TYPE_STRING);

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
KlelFindBasename(KLEL_VALUE **asArgs, void *pvData)
{
  char acBuffer[FILENAME_MAX + 1];
  char *pcTemp = NULL;

  KLEL_ASSERT(asArgs           != NULL);
  KLEL_ASSERT(asArgs[0]        != NULL);
  KLEL_ASSERT(asArgs[1]        == NULL);
  KLEL_ASSERT(asArgs[0]->iType == KLEL_TYPE_STRING);

  strncpy(acBuffer, asArgs[0]->acString, FILENAME_MAX);
  pcTemp = basename(acBuffer);
  return KlelCreateString(strlen(pcTemp), pcTemp);
}


/*-
 ***********************************************************************
 *
 * GetTypeOfVar
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE
GetTypeOfVar(const char *pcName, void *pvData)
{
  size_t         szi       = 0;
  KLEL_TYPE_SPEC asTypes[] =
  {
    {"basename",      KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_STRING)},
    {"defined",       KLEL_TYPE_BOOLEAN_FUNCTION1(KLEL_TYPE_STRING)},
    {"path",          KLEL_TYPE_STRING},
    {"mode",          KLEL_TYPE_INT64},
    {"uid",           KLEL_TYPE_INT64},
    {"gid",           KLEL_TYPE_INT64},
    {"owner",         KLEL_TYPE_STRING},
    {"group",         KLEL_TYPE_STRING},
    {"size",          KLEL_TYPE_INT64},
    {"atime",         KLEL_TYPE_INT64},
    {"mtime",         KLEL_TYPE_INT64},
    {"ctime",         KLEL_TYPE_INT64},
    {"directory",     KLEL_TYPE_BOOLEAN},
    {"device",        KLEL_TYPE_BOOLEAN},
    {"block_device",  KLEL_TYPE_BOOLEAN},
    {"char_device",   KLEL_TYPE_BOOLEAN},
    {"fifo",          KLEL_TYPE_BOOLEAN},
    {"inode",         KLEL_TYPE_INT64},
    {"rdev",          KLEL_TYPE_INT64},
    {"suid",          KLEL_TYPE_BOOLEAN},
    {"sgid",          KLEL_TYPE_BOOLEAN},
    {"sticky",        KLEL_TYPE_BOOLEAN},
    {"u_r",           KLEL_TYPE_BOOLEAN},
    {"u_w",           KLEL_TYPE_BOOLEAN},
    {"u_x",           KLEL_TYPE_BOOLEAN},
    {"g_r",           KLEL_TYPE_BOOLEAN},
    {"g_w",           KLEL_TYPE_BOOLEAN},
    {"g_x",           KLEL_TYPE_BOOLEAN},
    {"o_r",           KLEL_TYPE_BOOLEAN},
    {"o_w",           KLEL_TYPE_BOOLEAN},
    {"o_x",           KLEL_TYPE_BOOLEAN}
  };

  for (szi = 0; szi < (sizeof(asTypes) / sizeof(KLEL_TYPE_SPEC)); szi++)
  {
    if (strcmp(asTypes[szi].pcName, pcName) == 0)
    {
      return asTypes[szi].iType;
    }
  }

  return KLEL_TYPE_UNKNOWN;
}


/*-
 ***********************************************************************
 *
 * GetValueOfVar
 *
 ***********************************************************************
 */
KLEL_VALUE *
GetValueOfVar(const char *pcName, void *pvData)
{
  char               acBuffer[FILENAME_MAX + 1];
  struct group      *psGroup  = NULL;
  struct passwd     *psPasswd = NULL;
  KLEL_FIND_ITEM    *psItem   = (KLEL_FIND_ITEM *)(KlelGetPrivateData(gpsContext));
  const char        *pcPath   = psItem->pcName;
  const struct stat *psStat   = psItem->psStat;

  KLEL_ASSERT(pcName != NULL);
  KLEL_ASSERT(pcPath != NULL);

  if (strcmp(pcName, "basename") == 0)
  {
    return KlelCreateFunction(KLEL_TYPE_STRING_FUNCTION1(KLEL_TYPE_STRING), "basename", KlelFindBasename);
  }
  else if (strcmp(pcName, "defined") == 0)
  {
    return KlelCreateFunction(KLEL_TYPE_BOOLEAN_FUNCTION1(KLEL_TYPE_STRING), "defined", KlelFindDefined);
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

  return NULL;
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
  pid_t          iPid       = 0;
  KLEL_VALUE     *psResult  = NULL;
  int            iStatus    = 0;
  KLEL_FIND_ITEM psItem     = {0};

  KLEL_ASSERT(pcName != NULL);

  psItem.pcName = pcName;
  psItem.psStat = psStat;
  KlelSetPrivateData(gpsContext, (void *)&psItem);

  psResult = KlelExecute(gpsContext);
  if (psResult == NULL)
  {
    fprintf(stderr, "expression failed for %s: %s\n", pcName, KlelGetFirstError(gpsContext));
  }

  if (psResult != NULL && psResult->bBoolean)
  {
    if (KlelIsGuardedCommand(gpsContext))
    {
      psCommand = KlelGetCommand(gpsContext);
      if (psCommand != NULL)
      {
        if (strcmp(psCommand->acInterpreter, "system") == 0)
        {
          iStatus = system(psCommand->acProgram);
          if (iStatus == -1)
          {
            perror("unable to execute command");
          }
        }
        else if (strcmp(psCommand->acInterpreter, "exec") == 0)
        {
          iPid = fork();
          if (iPid == 0)
          {
            if (execv(psCommand->acProgram, psCommand->ppcArgumentVector) != 0)
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

        if (!KlelIsSuccessReturnCode(gpsContext, WEXITSTATUS(iStatus)))
        {
          fprintf(stderr, "guard matched for path %s, but command exited with status %d\n", pcName, WEXITSTATUS(iStatus));
        }

        KlelFreeCommand(psCommand);
      }
      else
      {
        fprintf(stderr, "guard matched for path %s, but command was invalid: %s\n", pcName, KlelGetFirstError(gpsContext));
      }
    }
    else
    {
      printf("%s\n", pcName);
    }
  }

  KlelFreeResult(psResult);

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
  const char *pcError = NULL;

  if (argc < 3)
  {
    fprintf(stderr, "\nUsage: %s <directory> <expression>\n\n", PROGRAM_NAME);
    return 1;
  }

  gpsContext = KlelCompile(argv[2], 0, GetTypeOfVar, GetValueOfVar, NULL);
  if (!KlelIsValid(gpsContext))
  {
    for (pcError = KlelGetFirstError(gpsContext); pcError != NULL; pcError = KlelGetNextError(gpsContext))
    {
      fprintf(stderr, "error: %s\n", pcError);
    }
    return 1;
  }

  if (KlelGetTypeOfExpression(gpsContext) != KLEL_TYPE_BOOLEAN)
  {
    fprintf(stderr, "error: %s requires predicates to be of Boolean type\n", argv[0]);
    return 1;
  }

  nftw(argv[1], ProcessDirectory, 2, 0);

  KlelFreeContext(gpsContext);

  return 0;
}
