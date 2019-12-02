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

/**@file   struct_probdata.h
 * @brief  problem structure of capacitated p-median example
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __CPMP_STRUCT_PROBDATA__
#define __CPMP_STRUCT_PROBDATA__

#include "scip/def.h"

/* capacitated p-median problem data */
struct SCIP_ProbData
{
   int                   nlocations;         /**< number of locations                                                   */
   int                   nclusters;          /**< number of clusters (the 'p')                                          */
   SCIP_Longint**        distances;          /**< distances between the locations, matrix of size nlocations*nlocations */
   SCIP_Longint*         demands;            /**< demands of the locations, vector of size nlocations                   */
   SCIP_Longint*         capacities;         /**< capacities of the locations, vector of size nlocations                */

   SCIP_CONS**           serviceconss;
   SCIP_CONS**           convconss;
   SCIP_CONS*            mediancons;
};

#endif
