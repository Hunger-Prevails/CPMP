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

/**@file   pricer_cpmp.h
 * @ingroup PRICERS
 * @brief  cpmp variable pricer
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __CPMP_PRICER_CPMP_H__
#define __CPMP_PRICER_CPMP_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

/** creates the cpmp variable pricer and includes it in SCIP */
EXTERN
SCIP_RETCODE SCIPincludePricerCpmp(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** forbid assignments for a certain location */
EXTERN
void SCIPpricerCpmpForbidAssignments(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   location,
   SCIP_Bool*            forbidden
   );

/** forbid assignments for a certain location */
EXTERN
void SCIPpricerCpmpForbidAssignment(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   median,
   int                   location
   );

/** allow previously forbidden assignments for a certain location */
EXTERN
void SCIPpricerCpmpAllowAssignments(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   location,
   SCIP_Bool*            forbidden
   );

/** allow assignments for a certain location */
EXTERN
void SCIPpricerCpmpAllowAssignment(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   median,
   int                   location
   );

/** check whether a certain assignment is currently forbidden */
EXTERN
SCIP_Bool SCIPpricerCpmpIsAssignmentForbidden(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   median,
   int                   location
   );

#ifdef __cplusplus
}
#endif

#endif
