/*-
 ***********************************************************************
 *
 * $Id: klel-expr.h,v 1.11 2019/07/31 15:59:27 klm Exp $
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
#ifndef _KLEL_EXPR_H_INCLUDED
#define _KLEL_EXPR_H_INCLUDED

/*-
 ***********************************************************************
 *
 * Defines
 *
 ***********************************************************************
 */
#define PROGRAM_NAME "klel-expr"

#define XER_OK 0
#define XER_Usage 1
#define XER_Setup 2
#define XER_Out_Of_Memory 3
#define XER_Execute 4
#define XER_GuardedCommand 5
#define XER_Timer 6

/*-
 ***********************************************************************
 *
 * Function Prototypes
 *
 ***********************************************************************
 */
KLEL_EXPR_TYPE GetTypeOfVar(const char *pcName,   void *pvData);
KLEL_VALUE     *GetValueOfVar(const char *pcName, void *pvData);

#endif /* !_KLEL_EXPR_H_INCLUDED */
