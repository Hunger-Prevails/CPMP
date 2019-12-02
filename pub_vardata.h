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

/**@file   pub_vardata.h
 * @brief  methods for accessing capacitated p-median problem variable data
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __CPMP_PUB_VARDATA__
#define __CPMP_PUB_VARDATA__

#include "scip/scip.h"

/* print the variable data */
extern
void SCIPprintVarData(
   SCIP*                 scip,
   SCIP_VAR*             var
   );

/* for a given solution, print the represented clusters */
extern
void SCIPprintSolClusters(
   SCIP*                 scip,
   SCIP_SOL*             sol
   );

/* get the median the variable belongs to */
extern
int SCIPvarGetMedian(
   SCIP_VAR*             var
   );

/* get the locations covered by the cluster represented by the variable */
extern
int* SCIPvarGetLocations(
   SCIP_VAR*             var
   );

/* get the number of locations covered by the cluster represented by the variable */
extern
int SCIPvarGetNLocations(
   SCIP_VAR*             var
   );

/* check if a given location is coverd by the cluster represented by the variable */
extern
SCIP_Bool SCIPisLocationInCluster(
   SCIP_VAR*             var,
   int                   location
   );

#endif
