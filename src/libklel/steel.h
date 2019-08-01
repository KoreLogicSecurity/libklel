/*-
 ***********************************************************************
 *
 * $Id: steel.h,v 1.5 2019/07/31 15:59:27 klm Exp $
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
#ifndef _STEEL_H_INCLUDED
#define _STEEL_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Typedefs
 *
 ***********************************************************************
 */
#ifndef _KLEL_STEEL
#define _KLEL_STEEL
typedef enum _KLEL_STRING_NODE_TYPE
{
  KLEL_STRING_NODE_LITERAL,
  KLEL_STRING_NODE_CONCAT,
  KLEL_STRING_NODE_EMPTY
} KLEL_STRING_NODE_TYPE;

typedef struct _KLEL_STRING_NODE
{
  KLEL_STRING_NODE_TYPE    iType;
  size_t                   szLength;
  int                      iRefCount;
  char                     *pcStringLiteral;
  struct _KLEL_STRING_NODE *psLeftSubNode;
  struct _KLEL_STRING_NODE *psRightSubNode;
} KLEL_STRING_NODE;
#endif

/*-
 ***********************************************************************
 *
 * Global variables
 *
 ***********************************************************************
 */
extern KLEL_STRING_NODE gsEmptyStringSingleton;

/*-
 ***********************************************************************
 *
 * Function prototypes
 *
 ***********************************************************************
 */
KLEL_STRING_NODE *SteelBuildSubString(KLEL_STRING_NODE *psString, int iStartIndex, int iEndIndex);
int               SteelCompareToNode(KLEL_STRING_NODE *psLeftString, KLEL_STRING_NODE *psRightString);
int               SteelCompareToPointer(KLEL_STRING_NODE *psStringNode, const char *pcString);
KLEL_STRING_NODE *SteelConcat(KLEL_STRING_NODE *psLeft, KLEL_STRING_NODE *psRight);
void              SteelCopyToCString(KLEL_STRING_NODE *psString, char *pcBuffer);
KLEL_STRING_NODE *SteelCreateFragment(size_t szLength, const char *pcTemp);
int               SteelDoCompareToNode(KLEL_STRING_NODE *psLeft, int iLeftOff, KLEL_STRING_NODE *psRight, int iRightOff);
int               SteelDoCompareToPointer(KLEL_STRING_NODE *psStringNode, const char *pcString, size_t szLength);
void              SteelFreeString(KLEL_STRING_NODE *psString);
size_t            SteelStringLength(KLEL_STRING_NODE *psString);
char             *SteelStringToCString(KLEL_STRING_NODE *psString);
KLEL_STRING_NODE *SteelSubString(KLEL_STRING_NODE *psString, int startIndex, int endIndex);

#endif /* !_STEEL_H_INCLUDED */
