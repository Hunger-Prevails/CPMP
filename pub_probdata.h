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

/**@file   pub_probdata.h
 * @brief  methods for accessing capacitated p-median problem data
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __CPMP_PUB_PROBDATA__
#define __CPMP_PUB_PROBDATA__

#include "scip/scip.h"

/** print the raw problem data */
extern
void SCIPprintProbData(
   SCIP*                 scip
   );

/** get number of locations */
extern
int SCIPprobdataGetNLocations(
   SCIP*                 scip
   );

/** get distances */
extern
SCIP_Longint** SCIPprobdataGetDistances(
   SCIP*                 scip
   );

/** get demands */
extern
SCIP_Longint* SCIPprobdataGetDemands(
   SCIP*                 scip
   );

/** get capacities */
extern
SCIP_Longint* SCIPprobdataGetCapacities(
   SCIP*                 scip
   );

/** get service constraints */
extern
SCIP_CONS** SCIPprobdataGetServiceconss(
   SCIP*                 scip
   );

/** get convexity constraints */
extern
SCIP_CONS** SCIPprobdataGetConvconss(
   SCIP*                 scip
   );

/** get convexity constraints */
extern
SCIP_CONS* SCIPprobdataGetMediancons(
   SCIP*                 scip
   );

#endif
