/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2018 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   cons_semiassign.h
 * @ingroup CONSHDLRS
 * @brief  constraint handler for semiassignment constraints
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __CPMP_CONS_SEMIASSIGN_H__
#define __CPMP_CONS_SEMIASSIGN_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

/** creates the handler for semiassign constraints and includes it in SCIP */
EXTERN
SCIP_RETCODE SCIPincludeConshdlrSemiassign(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** creates and captures a semiassign constraint
 *
 *  @note the constraint gets captured, hence at one point you have to release it using the method SCIPreleaseCons()
 */
EXTERN
SCIP_RETCODE SCIPcreateConsSemiassign(
   SCIP*                 scip,               /**< SCIP data structure                                                             */
   SCIP_CONS**           cons,               /**< pointer to hold the created constraint                                          */
   const char*           name,               /**< name of constraint                                                              */
   int                   location,           /**< location for which certain medians are forbidden                                */
   SCIP_Bool*            forbidden,          /**< for each median, the information whether the location may not be assigned to it */
   SCIP_NODE*            node                /**< node for which the constraint is valid                                          */
   );

#ifdef __cplusplus
}
#endif

#endif
