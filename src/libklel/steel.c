/*-
 ***********************************************************************
 *
 * $Id: steel.c,v 1.6 2019/07/31 15:59:27 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2013-2019 The KL-EL Project, All Rights Reserved.
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
 * Global variables
 *
 ***********************************************************************
 */
KLEL_STRING_NODE gsEmptyStringSingleton = {KLEL_STRING_NODE_EMPTY, 0, -1, NULL, NULL, NULL};


/*-
 ***********************************************************************
 *
 * SteelBuildSubString
 *
 ***********************************************************************
 */
KLEL_STRING_NODE *
SteelBuildSubString(KLEL_STRING_NODE *psString, int iStartIndex, int iEndIndex)
{
  KLEL_STRING_NODE *psLeft = NULL, *psRight = NULL, *psResult = NULL;

  if (psString->iType == KLEL_STRING_NODE_LITERAL)
  {
    return SteelCreateFragment((iEndIndex - iStartIndex > 0) ? iEndIndex - iStartIndex : 1, psString->pcStringLiteral + iStartIndex);
  }
  else if (psString->iType == KLEL_STRING_NODE_CONCAT)
  {
    if (iStartIndex <= psString->psLeftSubNode->szLength - 1)
    {
      psLeft = SteelBuildSubString
      (
        psString->psLeftSubNode,
        iStartIndex,
        (iEndIndex > psString->psLeftSubNode->szLength - 1) ? psString->psLeftSubNode->szLength - 1 : iEndIndex
      );
    }

    if (iEndIndex > psString->psLeftSubNode->szLength - 1)
    {
      psRight = SteelBuildSubString
      (
        psString->psRightSubNode,
        0,
        iEndIndex - psString->psLeftSubNode->szLength
      );
    }

    if (psLeft != NULL && psRight != NULL)
    {
      return SteelConcat(psLeft, psRight);
    }
    else
    {
      return (psLeft == NULL) ? psRight : psLeft;
    }
  }

/* FIXME What is the proper return value at this point? */
  KLEL_ASSERT(0);
  return NULL;
}


/*-
 ***********************************************************************
 *
 * SteelCompareToNode
 *
 ***********************************************************************
 */
int
SteelCompareToNode(KLEL_STRING_NODE *psLeftString, KLEL_STRING_NODE *psRightString)
{
  if (psLeftString == NULL || psRightString == NULL)
  {
    return (psLeftString == NULL) ? (psRightString == NULL) ? 0 : -1 : 1;
  }

  if (psLeftString->szLength != psRightString->szLength)
  {
    return (psLeftString->szLength > psRightString->szLength) ? 1 : -1;
  }

  if (psLeftString->iType == KLEL_STRING_NODE_LITERAL && psRightString->iType == KLEL_STRING_NODE_LITERAL)
  {
    return strncmp(psLeftString->pcStringLiteral, psRightString->pcStringLiteral, psLeftString->szLength);
  }
  else
  {
    return SteelDoCompareToNode(psLeftString, 0, psRightString, 0);
  }
}


/*-
 ***********************************************************************
 *
 * SteelCompareToPointer
 *
 ***********************************************************************
 */
int
SteelCompareToPointer(KLEL_STRING_NODE *psStringNode, const char *pcString)
{
  size_t szLength = 0;

  if (psStringNode == NULL || pcString == NULL)
  {
    return (psStringNode == NULL) ? (pcString == NULL) ? 0 : -1 : 1;
  }

  szLength = strlen(pcString);
  if (psStringNode->szLength != szLength)
  {
    return (psStringNode->szLength > szLength) ? 1 : -1;
  }

  if (psStringNode->iType == KLEL_STRING_NODE_LITERAL)
  {
    return strncmp(psStringNode->pcStringLiteral, pcString, szLength);
  }
  else
  {
    return SteelDoCompareToPointer(psStringNode, pcString, szLength);
  }
}


/*-
 ***********************************************************************
 *
 * SteelConcat
 *
 ***********************************************************************
 */
KLEL_STRING_NODE *
SteelConcat(KLEL_STRING_NODE *psLeft, KLEL_STRING_NODE *psRight)
{
  KLEL_STRING_NODE *psTemp = NULL;

  if (psLeft->iType == KLEL_STRING_NODE_EMPTY)
  {
    psRight->iRefCount++;
    return psRight;
  }

  if (psRight->iType == KLEL_STRING_NODE_EMPTY)
  {
    psLeft->iRefCount++;
    return psLeft;
  }

  psTemp = calloc(1, sizeof(KLEL_STRING_NODE));

  if (psTemp != NULL)
  {
    psTemp->iType = KLEL_STRING_NODE_CONCAT;
    psTemp->szLength = psLeft->szLength + psRight->szLength;
    psTemp->iRefCount = 1;
    psLeft->iRefCount++;
    psRight->iRefCount++;
    psTemp->psLeftSubNode = psLeft;
    psTemp->psRightSubNode = psRight;
  }

  return psTemp;
}


/*-
 ***********************************************************************
 *
 * SteelCopyToCString
 *
 ***********************************************************************
 */
void
SteelCopyToCString(KLEL_STRING_NODE *psString, char *pcBuffer)
{
  if (psString->iType == KLEL_STRING_NODE_CONCAT)
  {
    SteelCopyToCString(psString->psLeftSubNode, pcBuffer);
    SteelCopyToCString(psString->psRightSubNode, pcBuffer + psString->psLeftSubNode->szLength);
  }
  else if (psString->iType == KLEL_STRING_NODE_LITERAL)
  {
    strncpy(pcBuffer, psString->pcStringLiteral, psString->szLength);
  }
}


/*-
 ***********************************************************************
 *
 * SteelCreateFragment
 *
 ***********************************************************************
 */
KLEL_STRING_NODE *
SteelCreateFragment(size_t szLength, const char *pcTemp)
{
  char *pcFragment = NULL;
  KLEL_STRING_NODE *psTempString = NULL;

  if (pcTemp == NULL)
  {
    if (szLength == 0)
    {
      return &gsEmptyStringSingleton;
    }
    else
    {
      return NULL;
    }
  }

  psTempString = calloc(1, sizeof(KLEL_STRING_NODE));

  if (psTempString != NULL)
  {
    psTempString->iType = KLEL_STRING_NODE_LITERAL;
    psTempString->szLength = szLength;
    psTempString->iRefCount = 1;

    if (szLength > 0)
    {
      pcFragment = calloc(1, szLength + 1);
      if (pcFragment == NULL)
      {
        free(psTempString);
        return NULL;
      }
      memcpy(pcFragment, pcTemp, szLength);
      psTempString->pcStringLiteral = pcFragment;
    }
  }

  return psTempString;
}


/*-
 ***********************************************************************
 *
 * SteelDoCompareToNode
 *
 ***********************************************************************
 */
int
SteelDoCompareToNode(KLEL_STRING_NODE *psLeft, int iLeftOff, KLEL_STRING_NODE *psRight, int iRightOff)
{
  KLEL_STRING_NODE *psLeftToComp = NULL, *psRightToComp = NULL;
  int iRetVal = 0, iLeftOffToComp = 0, iRightOffToComp = 0;
  size_t szLength = 0;

  if (psLeft->iType == KLEL_STRING_NODE_LITERAL && psRight->iType == KLEL_STRING_NODE_LITERAL)
  {
    szLength = (psLeft->szLength - iLeftOff > psRight->szLength - iRightOff) ? psRight->szLength - iRightOff : psLeft->szLength - iLeftOff;
    return strncmp(psLeft->pcStringLiteral + iLeftOff, psRight->pcStringLiteral + iRightOff, szLength);
  }

  if (psLeft->iType == KLEL_STRING_NODE_CONCAT)
  {
    if (iLeftOff < psLeft->psLeftSubNode->szLength)
    {
      psLeftToComp = psLeft->psLeftSubNode;
      iLeftOffToComp = iLeftOff;
    }
    else
    {
      psLeftToComp = psLeft->psRightSubNode;
      iLeftOffToComp = iLeftOff - psLeft->psLeftSubNode->szLength;
    }
  }

  if (psRight->iType == KLEL_STRING_NODE_CONCAT)
  {
    if (iRightOff < psRight->psLeftSubNode->szLength)
    {
      psRightToComp = psRight->psLeftSubNode;
      iRightOffToComp = iRightOff;
    }
    else
    {
      psRightToComp = psRight->psRightSubNode;
      iRightOffToComp = iRightOff - psRight->psLeftSubNode->szLength;
    }
  }

  if (psLeftToComp != NULL)
  {
    if (psRightToComp != NULL)
    {
      while(psLeftToComp != psLeft->psRightSubNode || psRightToComp != psRight->psRightSubNode)
      {
        iRetVal = SteelDoCompareToNode(psLeftToComp, iLeftOffToComp, psRightToComp, iRightOffToComp);
        if (iRetVal != 0)
        {
          return iRetVal;
        }

        if (psLeftToComp->szLength - iLeftOffToComp < psRightToComp->szLength - iRightOffToComp)
        {
          iRightOffToComp += psLeftToComp->szLength - iLeftOffToComp;
          iLeftOffToComp = 0;
          psLeftToComp = psLeft->psRightSubNode;
        }
        else if (psLeftToComp->szLength - iLeftOffToComp > psRightToComp->szLength - iRightOffToComp)
        {
          iLeftOffToComp += psRightToComp->szLength - iRightOffToComp;
          iRightOffToComp = 0;
          psRightToComp = psRight->psRightSubNode;
        }
        else
        {
          iLeftOffToComp = 0;
          iRightOffToComp = 0;
          psLeftToComp = psLeft->psRightSubNode;
          psRightToComp = psRight->psRightSubNode;
        }
      }

      return SteelDoCompareToNode(psLeftToComp, iLeftOffToComp, psRightToComp, iRightOffToComp);
    }
    else
    {
      iRetVal = SteelDoCompareToNode(psLeftToComp, iLeftOffToComp, psRight, iRightOff);
      if (iRetVal != 0)
      {
        return iRetVal;
      }

      iRightOffToComp = iRightOff + psLeftToComp->szLength;
      psLeftToComp = psLeft->psRightSubNode;
      iLeftOffToComp = 0;
      return SteelDoCompareToNode(psLeftToComp, iLeftOffToComp, psRight, iRightOffToComp);
    }
  }
  else
  {
    iRetVal = SteelDoCompareToNode(psLeft, iLeftOff, psRightToComp, iRightOffToComp);
    if ( iRetVal != 0)
    {
      return iRetVal;
    }

    iLeftOffToComp = iLeftOff + psRightToComp->szLength;
    psRightToComp = psRight->psRightSubNode;
    iRightOffToComp = 0;
    return SteelDoCompareToNode(psLeft, iLeftOffToComp, psRightToComp, iRightOffToComp);
  }

  return SteelDoCompareToNode
  (
    (psLeftToComp != NULL)  ? psLeftToComp    : psLeft,
    (psLeftToComp != NULL)  ? iLeftOffToComp  : iLeftOff,
    (psRightToComp != NULL) ? psRightToComp   : psRight,
    (psRightToComp != NULL) ? iRightOffToComp : iRightOff
  );
}


/*-
 ***********************************************************************
 *
 * SteelDoCompareToPointer
 *
 ***********************************************************************
 */
int
SteelDoCompareToPointer(KLEL_STRING_NODE *psStringNode, const char *pcString, size_t szLength)
{
  int iLeftResult = 0;

  if (psStringNode->iType == KLEL_STRING_NODE_LITERAL)
  {
    return strncmp(psStringNode->pcStringLiteral, pcString, szLength);
  }

  else if (psStringNode->iType == KLEL_STRING_NODE_CONCAT)
  {
    iLeftResult = SteelDoCompareToPointer(psStringNode->psLeftSubNode, pcString, psStringNode->psLeftSubNode->szLength);
    return (iLeftResult != 0)
      ? iLeftResult
      : SteelDoCompareToPointer(psStringNode->psRightSubNode, pcString + psStringNode->psLeftSubNode->szLength, psStringNode->psRightSubNode->szLength);
  }

/* FIXME What is the proper return value at this point? */
  KLEL_ASSERT(0);
  return 0;
}


/*-
 ***********************************************************************
 *
 * SteelFreeString
 *
 ***********************************************************************
 */
void
SteelFreeString(KLEL_STRING_NODE *psString)
{
  if(psString != NULL && psString->iType != KLEL_STRING_NODE_EMPTY)
  {
    psString->iRefCount--;

    if (psString->iRefCount == 0)
    {
      if (psString->iType == KLEL_STRING_NODE_CONCAT)
      {
        SteelFreeString(psString->psLeftSubNode);
        SteelFreeString(psString->psRightSubNode);
      }
      else
      {
        free(psString->pcStringLiteral);
      }
      free(psString);
    }
  }
}


/*-
 ***********************************************************************
 *
 * SteelStringLength
 *
 ***********************************************************************
 */
size_t
SteelStringLength(KLEL_STRING_NODE *psString)
{
  if (psString != NULL)
  {
    return psString->szLength;
  }
  else
  {
    return 0;
  }
}


/*-
 ***********************************************************************
 *
 * SteelStringToCString
 *
 ***********************************************************************
 */
char *
SteelStringToCString(KLEL_STRING_NODE *psString)
{
  char *pcTemp = NULL;

  if (psString == NULL)
  {
    return NULL;
  }

  pcTemp = calloc(1, psString->szLength + 1);
  if (pcTemp != NULL && psString->szLength != 0)
  {
    SteelCopyToCString(psString, pcTemp);
  }

  return pcTemp;
}


/*-
 ***********************************************************************
 *
 * SteelSubString
 *
 ***********************************************************************
 */
KLEL_STRING_NODE *
SteelSubString(KLEL_STRING_NODE *psString, int iStartIndex, int iEndIndex)
{
  if (iStartIndex > iEndIndex || psString == NULL)
  {
    return NULL;
  }

  if (iStartIndex < 0)
  {
    iStartIndex = 0;
  }
  if (iEndIndex > psString->szLength - 1)
  {
    iEndIndex = psString->szLength - 1;
  }

  if (psString->iType == KLEL_STRING_NODE_EMPTY)
  {
    return psString;
  }

  return SteelBuildSubString(psString, iStartIndex, iEndIndex);
}
