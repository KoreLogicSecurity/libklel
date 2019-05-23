/*-
 ***********************************************************************
 *
 * $Id: klel-expr.h,v 1.8 2012/11/21 20:29:53 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
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
