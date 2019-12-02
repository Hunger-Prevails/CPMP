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

/**@file   pricer_cpmp.c
 * @brief  variable pricer for capacitated p-median problems
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "pricer_cpmp.h"
#include "pub_probdata.h"
#include "pub_vardata.h"
#include "vardata.h"

#include "scip/cons_linear.h"
#include "scip/cons_knapsack.h"


#define PRICER_NAME            "cpmp"
#define PRICER_DESC            "variable pricer for capacitated p-median problems"
#define PRICER_PRIORITY        0
#define PRICER_DELAY           TRUE     /* only call pricer if all problem variables have non-negative reduced costs */




/*
 * Data structures
 */

/** variable pricer data */
struct SCIP_PricerData
{
   SCIP_Bool**           forbiddenassignments; /* matrix of assignments which are forbidden by the current branching decisions */
};




/*
 * Local methods
 */


/**
 * add a new column to the master problem
 */
static
SCIP_RETCODE addColumn(
   SCIP*                 scip,               /* SCIP data structure                                  */
   int                   median,             /* median for which the pricing problem has been solved */
   int*                  locations,          /* locations contained in the new cluster               */
   int                   nlocations,         /* number of locations                                  */
   SCIP_Real             score               /* score for the column: either its reduces cost or Farkas value */
   )
{
   SCIP_Longint** distances;
   SCIP_CONS** serviceconss;
   SCIP_CONS** convconss;
   SCIP_CONS* mediancons;

   SCIP_VAR* var;
   char name[SCIP_MAXSTRLEN];
   SCIP_Real cost;

   int i;

   /* get necessary problem data */
   distances = SCIPprobdataGetDistances(scip);
   serviceconss = SCIPprobdataGetServiceconss(scip);
   convconss = SCIPprobdataGetConvconss(scip);
   mediancons = SCIPprobdataGetMediancons(scip);

   assert(distances != NULL);
   assert(serviceconss != NULL);
   assert(convconss != NULL);
   assert(mediancons != NULL);

   /* ****************************************************************************************************
    * TODO: compute the total service costs of the new cluster, to be stored in 'cost'
    * ****************************************************************************************************
    */



   /* create a new variable representing the found cluster, add the corresponding data and add it to the master problem */
   (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "column_%d", SCIPgetNVars(scip));
   SCIP_CALL( SCIPcreateVarBasic(scip, &var, name, 0.0, 1.0, cost, SCIP_VARTYPE_INTEGER) );
   SCIP_CALL( SCIPcreateVarData(scip, var, median, locations, nlocations) );
   SCIP_CALL( SCIPaddPricedVar(scip, var, score) );
   SCIP_CALL( SCIPchgVarUbLazy(scip, var, 1.0) );


   /* ****************************************************************************************************
    * TODO: add the variable to the master constraints:
    *   * to each service constraints whose location is contained in the cluster;
    *   * to the convexity constraint of the median
    *   * to the p-median constraint
    * ****************************************************************************************************
    */


   SCIPdebugMessage("Found improving column, score=%g:\n", score);
   SCIPdebug( SCIPprintVarData(scip, var) );

   SCIP_CALL( SCIPreleaseVar(scip, &var) );

   return SCIP_OKAY;
}


/**
 * Call the pricing routine
 */
static
SCIP_RETCODE performPricing(
   SCIP*                 scip,               /* SCIP data structure                                  */
   SCIP_PRICERDATA*      pricerdata,         /* pricer data structure                                */
   SCIP_Bool             useredcost,         /* Is reduced cost pricing or Farkas pricing performed? */
   SCIP_RESULT*          result              /* SCIP result pointer                                  */
   )
{
   int nlocations;
   SCIP_Longint** distances;
   SCIP_Longint* alldemands;
   SCIP_Longint* capacities;
   SCIP_CONS** serviceconss;
   SCIP_CONS** convconss;
   SCIP_CONS* mediancons;

   int* items;                               /* array of items in the knapsack problem                                            */
   int nitems;                               /* number of items                                                                   */
   SCIP_Real* profits;                       /* array of item profits                                                             */
   SCIP_Longint* demands;                    /* array of item demands                                                             */
   int* solitems;                            /* array of items contained in the knapsack; to be filled by the knapsack solver     */
   int nsolitems;                            /* number of items contained in the knapsack; to be set by the knapsack solver       */
   int* nonsolitems;                         /* array of items not contained in the knapsack; to be filled by the knapsack solver */
   int nnonsolitems;                         /* number of items not contained in the knapsack; to be set by the knapsack solver   */
   SCIP_Real solval;                         /* total profit returned by the knapsack solver                                      */
   SCIP_Bool success;                        /* indicates whether the knapsack problem was solved successfully                    */

   int median;
   int location;

   /* get necessary problem data */
   nlocations = SCIPprobdataGetNLocations(scip);
   distances = SCIPprobdataGetDistances(scip);
   alldemands = SCIPprobdataGetDemands(scip);
   capacities = SCIPprobdataGetCapacities(scip);
   serviceconss = SCIPprobdataGetServiceconss(scip);
   convconss = SCIPprobdataGetConvconss(scip);
   mediancons = SCIPprobdataGetMediancons(scip);

   assert(nlocations >= 0);
   assert(distances != NULL);
   assert(alldemands != NULL);
   assert(capacities != NULL);
   assert(serviceconss != NULL);
   assert(convconss != NULL);
   assert(mediancons != NULL);

   /* allocate memory */
   SCIP_CALL( SCIPallocBufferArray(scip, &items, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &profits, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &demands, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solitems, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nonsolitems, nlocations) );

   *result = SCIP_DIDNOTRUN;

   for( median = 0; median < nlocations && !SCIPisStopped(scip); ++median )
   {
      nitems = 0;

      /* ****************************************************************************************************
       * TODO: prepare the knapsack problem for the current pricing problem:
       * store the possible locations as items, get their demands and calculate their profits;
       *
       * NOTE: Due to branching restrictions, not all locations are added as items, so you
       * need to check the pricerdata->forbiddenassignments array
       *
       * NOTE: The profits depend on whether you do reduced cost pricing or Farkas pricing!
       * ****************************************************************************************************
       */


      SCIP_CALL( SCIPsolveKnapsackExactly(scip, nitems, demands, profits, capacities[median], items, solitems, nonsolitems, &nsolitems, &nnonsolitems, &solval, &success) );

      if( success )
      {
         SCIP_Real score;

         *result = SCIP_SUCCESS;

         /* ****************************************************************************************************
          * TODO: now that a column has been calculated, calculate its reduced cost or Farkas coefficient
          * (to be stored in 'score'); then, check whether it can be added and if so, use the addColumn()
          * method to add it to the master problem
          * ****************************************************************************************************
          */

         /* calculate the reduced cost or Farkas value of the new column */

         SCIPdebugMessage("  -> obj = %g\n", score);

         /* If an improving column has been found, add it */
         if( TRUE )
         {

         }
      }
      else
      {
         SCIPwarningMessage(scip, "Pricing problem for median %d could not be solved!\n", median+1);
      }
   }

   SCIPfreeBufferArray(scip, &nonsolitems);
   SCIPfreeBufferArray(scip, &solitems);
   SCIPfreeBufferArray(scip, &demands);
   SCIPfreeBufferArray(scip, &profits);
   SCIPfreeBufferArray(scip, &items);

   return SCIP_OKAY;
}

/*
 * Callback methods of variable pricer
 */

/** destructor of variable pricer to free user data (called when SCIP is exiting) */
static
SCIP_DECL_PRICERFREE(pricerFreeCpmp)
{  /*lint --e{715}*/
   SCIP_PRICERDATA* pricerdata;

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   SCIPfreeMemory(scip, &pricerdata);
   SCIPpricerSetData(pricer, NULL);

   return SCIP_OKAY;
}


/** solving process initialization method of variable pricer (called when branch and bound process is about to begin) */
static
SCIP_DECL_PRICERINITSOL(pricerInitsolCpmp)
{  /*lint --e{715}*/
   SCIP_PRICERDATA* pricerdata;
   int nlocations;
   int i;

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   nlocations = SCIPprobdataGetNLocations(scip);

   SCIP_CALL( SCIPallocMemoryArray(scip, &pricerdata->forbiddenassignments, nlocations) );
   for( i = 0; i < nlocations; ++i )
   {
      SCIP_CALL( SCIPallocMemoryArray(scip, &pricerdata->forbiddenassignments[i], nlocations) );
      BMSclearMemoryArray(pricerdata->forbiddenassignments[i], nlocations);
   }

   return SCIP_OKAY;
}


/** solving process deinitialization method of variable pricer (called before branch and bound process data is freed) */
static
SCIP_DECL_PRICEREXITSOL(pricerExitsolCpmp)
{  /*lint --e{715}*/
   SCIP_PRICERDATA* pricerdata;
   int nlocations;
   int i;

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   nlocations = SCIPprobdataGetNLocations(scip);

   for( i = 0; i < nlocations; ++i )
   {
      SCIPfreeMemoryArray(scip, &pricerdata->forbiddenassignments[i]);
   }
   SCIPfreeMemoryArray(scip, &pricerdata->forbiddenassignments);

   return SCIP_OKAY;
}


/** reduced cost pricing method of variable pricer for feasible LPs */
static
SCIP_DECL_PRICERREDCOST(pricerRedcostCpmp)
{  /*lint --e{715}*/
   SCIP_PRICERDATA* pricerdata;

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   SCIP_CALL( performPricing(scip, pricerdata, TRUE, result) );

   return SCIP_OKAY;
}


/** Farkas pricing method of variable pricer for infeasible LPs */
static
SCIP_DECL_PRICERFARKAS(pricerFarkasCpmp)
{  /*lint --e{715}*/
   SCIP_PRICERDATA* pricerdata;

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   SCIP_CALL( performPricing(scip, pricerdata, FALSE, result) );

   return SCIP_OKAY;
}




/*
 * variable pricer specific interface methods
 */

/** creates the cpmp variable pricer and includes it in SCIP */
SCIP_RETCODE SCIPincludePricerCpmp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_PRICERDATA* pricerdata;
   SCIP_PRICER* pricer;

   /* create cpmp variable pricer data */
   pricerdata = NULL;
   SCIP_CALL( SCIPallocMemory(scip, &pricerdata) );
   assert(pricerdata != NULL);

   /* include variable pricer */
   pricer = NULL;
   SCIP_CALL( SCIPincludePricerBasic(scip, &pricer, PRICER_NAME, PRICER_DESC, PRICER_PRIORITY, PRICER_DELAY,
         pricerRedcostCpmp, pricerFarkasCpmp, pricerdata) );
   assert(pricer != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetPricerFree(scip, pricer, pricerFreeCpmp) );
   SCIP_CALL( SCIPsetPricerInitsol(scip, pricer, pricerInitsolCpmp) );
   SCIP_CALL( SCIPsetPricerExitsol(scip, pricer, pricerExitsolCpmp) );

   return SCIP_OKAY;
}

/** forbid assignments for a certain location */
void SCIPpricerCpmpForbidAssignments(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   location,
   SCIP_Bool*            forbidden
   )
{
   SCIP_PRICER* pricer;
   SCIP_PRICERDATA* pricerdata;
   int nlocations;

   int median;

   pricer = SCIPfindPricer(scip, PRICER_NAME);
   assert(pricer != NULL);

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   nlocations = SCIPprobdataGetNLocations(scip);

   for( median = 0; median < nlocations; ++median )
      if( forbidden[median] )
         pricerdata->forbiddenassignments[median][location] = TRUE;

   return;
}

/** forbid assignments for a certain location */
void SCIPpricerCpmpForbidAssignment(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   median,
   int                   location
   )
{
   SCIP_PRICER* pricer;
   SCIP_PRICERDATA* pricerdata;

   pricer = SCIPfindPricer(scip, PRICER_NAME);
   assert(pricer != NULL);

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   assert(location < SCIPprobdataGetNLocations(scip) && location >= 0);
   assert(median < SCIPprobdataGetNLocations(scip) && median >= 0);

   pricerdata->forbiddenassignments[median][location] = TRUE;

   return;
}


/** allow previously forbidden assignments for a certain location */
void SCIPpricerCpmpAllowAssignments(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   location,
   SCIP_Bool*            forbidden
   )
{
   SCIP_PRICER* pricer;
   SCIP_PRICERDATA* pricerdata;
   int nlocations;

   int median;

   pricer = SCIPfindPricer(scip, PRICER_NAME);
   assert(pricer != NULL);

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   nlocations = SCIPprobdataGetNLocations(scip);

   for( median = 0; median < nlocations; ++median )
      if( forbidden[median] )
         pricerdata->forbiddenassignments[median][location] = FALSE;

   return;
}

/** allow assignments for a certain location */
void SCIPpricerCpmpAllowAssignment(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   median,
   int                   location
   )
{
   SCIP_PRICER* pricer;
   SCIP_PRICERDATA* pricerdata;

   pricer = SCIPfindPricer(scip, PRICER_NAME);
   assert(pricer != NULL);

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   assert(location < SCIPprobdataGetNLocations(scip) && location >= 0);
   assert(median < SCIPprobdataGetNLocations(scip) && median >= 0);

   pricerdata->forbiddenassignments[median][location] = FALSE;

   return;
}
/** check whether a certain assignment is currently forbidden */
SCIP_Bool SCIPpricerCpmpIsAssignmentForbidden(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   median,
   int                   location
   )
{
   SCIP_PRICER* pricer;
   SCIP_PRICERDATA* pricerdata;

   pricer = SCIPfindPricer(scip, PRICER_NAME);
   assert(pricer != NULL);

   pricerdata = SCIPpricerGetData(pricer);
   assert(pricerdata != NULL);

   return pricerdata->forbiddenassignments[median][location];
}
