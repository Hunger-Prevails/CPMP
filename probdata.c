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

/**@file   probdata.c
 * @brief  methods for handling capacitated p-median problem data
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <stdio.h>
#include "probdata.h"
#include "pub_probdata.h"
#include "struct_probdata.h"

#include "scip/cons_linear.h"
#include "scip/cons_setppc.h"


/** create problem data */
static
SCIP_RETCODE createProbData(
   SCIP*                 scip,
   SCIP_PROBDATA**       probdata,
   int                   nlocations,
   int                   nclusters,
   SCIP_Longint**        distances,
   SCIP_Longint*         demands,
   SCIP_Longint*         capacities
   )
{
   int i;

   assert(scip != NULL);

   /* allocate memory */
   *probdata = NULL;
   SCIP_CALL( SCIPallocMemory(scip, probdata) );
   assert(probdata != NULL);

   /* copy problem data */

   (*probdata)->nlocations = nlocations;
   (*probdata)->nclusters = nclusters;

   SCIP_CALL( SCIPallocMemoryArray(scip, &(*probdata)->distances, nlocations) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &(*probdata)->serviceconss, nlocations) );
   SCIP_CALL( SCIPallocMemoryArray(scip, &(*probdata)->convconss, nlocations) );
   for( i = 0; i < nlocations; ++i )
   {
      SCIP_CALL( SCIPduplicateMemoryArray(scip, &(*probdata)->distances[i], distances[i], nlocations) );
      (*probdata)->serviceconss[i] = NULL;
      (*probdata)->convconss[i] = NULL;
   }
   SCIP_CALL( SCIPduplicateMemoryArray(scip, &(*probdata)->demands, demands, nlocations) );
   SCIP_CALL( SCIPduplicateMemoryArray(scip, &(*probdata)->capacities, capacities, nlocations) );

   (*probdata)->mediancons = NULL;

   return SCIP_OKAY;
}


/** free problem data */
static
SCIP_RETCODE freeProbData(
   SCIP*                 scip,
   SCIP_PROBDATA**       probdata
   )
{
   int i;

   assert(scip != NULL);
   assert(probdata != NULL);

   /* free problem data */
   SCIP_CALL( SCIPreleaseCons(scip, &(*probdata)->mediancons) );
   SCIPfreeMemoryArray(scip, &(*probdata)->capacities);
   SCIPfreeMemoryArray(scip, &(*probdata)->demands);
   for( i = 0; i < (*probdata)->nlocations; ++i )
   {
      SCIP_CALL( SCIPreleaseCons(scip, &(*probdata)->convconss[i]) );
      SCIP_CALL( SCIPreleaseCons(scip, &(*probdata)->serviceconss[i]) );
      SCIPfreeMemoryArray(scip, &(*probdata)->distances[i]);
   }
   SCIPfreeMemoryArray(scip, &(*probdata)->convconss);
   SCIPfreeMemoryArray(scip, &(*probdata)->serviceconss);
   SCIPfreeMemoryArray(scip, &(*probdata)->distances);

   /* free probdata structure */
   SCIPfreeMemory(scip, probdata);

   return SCIP_OKAY;
}


/** create constraints (initially without any variables) */
static
SCIP_RETCODE createConstraints(
   SCIP*                 scip,
   SCIP_PROBDATA*        probdata,
   int                   nclusters,
   int                   nlocations
   )
{
   char name[SCIP_MAXSTRLEN];

   /* ****************************************************************************************************
    * TODO: create the master constraints; you need the following:
    *   * probdata->serviceconss: for each location, at least one cluster containing it must be chosen
    *   * probdata->convconss: each location can be the median of at most one cluster
    *   * probdata->mediancons: form exactly nclusters many clusters (i.e. select nclusters many medians)
    *
    * NOTE: The constraints do not contain any variable yet
    * ****************************************************************************************************
    */

   for (int i = 0;i < nlocations; ++i) {
		SCIPsnprintf(name, 13, "serviceconss" + (i + '0'));
		SCIP_CALL(SCIPcreateConsLinear(scip, &(probdata->serviceconss[i]), name, 0, NULL, NULL, 1, SCIPinfinity(scip), 1, 1, 1, 1, 1, 0, 1, 0, 0, 0));
		SCIP_CALL(SCIPaddCons(scip, probdata->serviceconss[i]));

		SCIPsnprintf(name, 13, "convconss" + (i + '0'));
		SCIP_CALL(SCIPcreateConsLinear(scip, &(probdata->convconss[i]), name, 0, NULL, NULL, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0));
		SCIP_CALL(SCIPaddCons(scip, probdata->convconss[i]));
   }

   SCIPsnprintf(name, 13, "mediancons");
   SCIP_CALL(SCIPcreateConsLinear(scip, &(probdata->mediancons), name, 0, NULL, NULL, 0, nclusters, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0));
   SCIP_CALL(SCIPaddCons(scip, probdata->mediancons));

   // SCIPprintProbData(scip);

   return SCIP_OKAY;
}


/** frees user data of original problem (called when the original problem is freed) */
static
SCIP_DECL_PROBDELORIG(probdataDelorig)
{
   SCIP_CALL( freeProbData(scip, probdata) );

   return SCIP_OKAY;
}


/** creates user data of transformed problem by transforming the original user problem data
 *  (called after problem was transformed)
 */
static
SCIP_DECL_PROBTRANS(probdataTrans)
{
   assert(scip != NULL);
   assert(sourcedata != NULL);

   SCIP_CALL( createProbData(scip, targetdata, sourcedata->nlocations, sourcedata->nclusters, sourcedata->distances, sourcedata->demands, sourcedata->capacities) );

   /* transform the constraints */
   SCIP_CALL( SCIPtransformConss(scip, sourcedata->nlocations, sourcedata->serviceconss, (*targetdata)->serviceconss) );
   SCIP_CALL( SCIPtransformConss(scip, sourcedata->nlocations, sourcedata->convconss, (*targetdata)->convconss) );
   SCIP_CALL( SCIPtransformCons(scip, sourcedata->mediancons, &(*targetdata)->mediancons) );

   return SCIP_OKAY;
}

/** frees user data of transformed problem (called when the transformed problem is freed)
 */
static
SCIP_DECL_PROBDELTRANS(probdataDeltrans)
{
   SCIP_CALL( freeProbData(scip, probdata) );

   return SCIP_OKAY;
}


/** create capacitated p-median SCIP instance and save the problem specific data */
SCIP_RETCODE SCIPcreateProbCpmp(
   SCIP*                 scip,
   int                   nlocations,
   int                   nclusters,
   SCIP_Longint**        distances,
   SCIP_Longint*         demands,
   SCIP_Longint*         capacities
   )
{
   SCIP_PRICER* pricer;
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   SCIP_CALL( createProbData(scip, &probdata, nlocations, nclusters, distances, demands, capacities) );

   /* notify SCIP about the data structure and set the destructors and transformation callback */
   SCIP_CALL( SCIPsetProbData(scip, probdata) );
   SCIP_CALL( SCIPsetProbDelorig(scip, probdataDelorig) );
   SCIP_CALL( SCIPsetProbTrans(scip, probdataTrans) );
   SCIP_CALL( SCIPsetProbDeltrans(scip, probdataDeltrans) );

   SCIP_CALL( createConstraints(scip, probdata, nclusters, nlocations) );

   /* activate the pricer in SCIP, such that SCIP calls it to price in new variables during LP solving */
   pricer = SCIPfindPricer(scip, "cpmp");
   assert(pricer != NULL);
   SCIP_CALL( SCIPactivatePricer(scip, pricer) );

   return SCIP_OKAY;
}


/** print the raw problem data */
void SCIPprintProbData(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;
   int i;
   int j;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   SCIPinfoMessage(scip, NULL, "\n");

   SCIPinfoMessage(scip, NULL, "nlocations  : %3d\n", probdata->nlocations);
   SCIPinfoMessage(scip, NULL, "nclusters   : %3d\n", probdata->nclusters);
   SCIPinfoMessage(scip, NULL, "\n");

   SCIPinfoMessage(scip, NULL, "distances   :\n");
   for( i = 0; i < probdata->nlocations; ++i )
   {
      SCIPinfoMessage(scip, NULL, "   ");
      for( j = 0; j < probdata->nlocations; ++j )
      {
         SCIPinfoMessage(scip, NULL, " %4"SCIP_LONGINT_FORMAT"", probdata->distances[i][j]);
      }
      SCIPinfoMessage(scip, NULL, "\n");
   }
   SCIPinfoMessage(scip, NULL, "\n");

   SCIPinfoMessage(scip, NULL, "demands     :");
   for( i = 0; i < probdata->nlocations; ++i )
   {
      SCIPinfoMessage(scip, NULL, " %4"SCIP_LONGINT_FORMAT"", probdata->demands[i]);
   }
   SCIPinfoMessage(scip, NULL, "\n");

   SCIPinfoMessage(scip, NULL, "capacities  :");
   for( i = 0; i < probdata->nlocations; ++i )
   {
      SCIPinfoMessage(scip, NULL, " %4"SCIP_LONGINT_FORMAT"", probdata->capacities[i]);
   }
   SCIPinfoMessage(scip, NULL, "\n");

   SCIPinfoMessage(scip, NULL, "\n");

   return;
}


/** get number of locations */
int SCIPprobdataGetNLocations(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->nlocations;
}


/** get distances */
SCIP_Longint** SCIPprobdataGetDistances(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->distances;
}


/** get demands */
SCIP_Longint* SCIPprobdataGetDemands(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->demands;
}


/** get capacities */
SCIP_Longint* SCIPprobdataGetCapacities(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->capacities;
}

/** get service constraints */
SCIP_CONS** SCIPprobdataGetServiceconss(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->serviceconss;
}

/** get convexity constraints */
SCIP_CONS** SCIPprobdataGetConvconss(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->convconss;
}

/** get convexity constraints */
SCIP_CONS* SCIPprobdataGetMediancons(
   SCIP*                 scip
   )
{
   SCIP_PROBDATA* probdata;

   assert(scip != NULL);

   probdata = SCIPgetProbData(scip);
   assert(probdata != NULL);

   return probdata->mediancons;
}
