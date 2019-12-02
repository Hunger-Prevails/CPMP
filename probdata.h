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

/**@file   probdata.h
 * @brief  methods for handling capacitated p-median problem data
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __CPMP_PROBDATA__
#define __CPMP_PROBDATA__

#include "scip/scip.h"

/** create capacitated p-median SCIP instance and save the problem specific data */
extern
SCIP_RETCODE SCIPcreateProbCpmp(
   SCIP*                 scip,
   int                   nlocations,
   int                   nclusters,
   SCIP_Longint**        distances,
   SCIP_Longint*         demands,
   SCIP_Longint*         capacities
   );

#endif
