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

/**@file   branch_semiassign.c
 * @ingroup BRANCHINGRULES
 * @brief  semi assignment branching rule
 * @author Christian Puchert
 * @author Jonas Witt
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include <string.h>

#include "branch_semiassign.h"
#include "cons_semiassign.h"
#include "pricer_cpmp.h"
#include "pub_probdata.h"
#include "pub_vardata.h"

/**@name Branching rule properties
 *
 * @{
 */

#define BRANCHRULE_NAME            "Semiassign"
#define BRANCHRULE_DESC            "semi assignment branching rule"
#define BRANCHRULE_PRIORITY        50000
#define BRANCHRULE_MAXDEPTH        -1
#define BRANCHRULE_MAXBOUNDDIST    1.0

/**@} */

/*
 * Local methods
 */

/** for each pair of locations, compute the (possibly fractional) assignment value */
static
SCIP_RETCODE computeAssignments(
   SCIP*                 scip,               /* SCIP data structure                                  */
   SCIP_SOL*             sol,                /* solution to be checked, or NULL for LP solution      */
   SCIP_Real**           assignments         /* matrix of location-median assignments                */
   )
{
   int nlocations;
   SCIP_VAR** vars;
   int nvars;

   int i;
   int j;

   SCIP_CALL( SCIPgetVarsData(scip, &vars, &nvars, NULL, NULL, NULL, NULL) );
   nlocations = SCIPprobdataGetNLocations(scip);

   for( i = 0; i < nlocations; ++i )
      for( j = 0; j < nlocations; ++j )
         assignments[i][j] = 0.0;
   
   /* ****************************************************************************************************
    * TODO: calculate the assignment value for each location-median pair.
    * i.e., assignments[i][j] should describe the (possible fractional) assignment value of
    * median j to location i.
    * This is done by looping over the master variables.
    *
    * NOTE: For implementational reasons reasons, the location is the *first* index here
    * and the median the *second*!
    * ****************************************************************************************************
    */
   for( i = 0; i < nvars; ++i )
   {
      int cluster = SCIPvarGetMedian(vars[i]);

      int nmembers = SCIPvarGetNLocations(vars[i]);

      int* memebers = SCIPvarGetLocations(vars[i]);

      for ( j = 0; j < nmembers; ++j ) assignments[memebers[j]][cluster] += SCIPgetSolVal(scip, sol, vars[i]);
   }
   return SCIP_OKAY;
}

/** for each location, sort the potential medians by nonincreasing value of fractional assignment */
static
void sortMedians(
   SCIP*                 scip,               /* SCIP data structure                                                     */
   int**                 sortedids,          /* for each location, the array of medians sorted by fractional assignment */
   SCIP_Real**           assignments         /* matrix of location-median assignments                                   */
   )
{
   int nlocations;

   int i;

   nlocations = SCIPprobdataGetNLocations(scip);

   for( i = 0; i < nlocations; ++i )
   {
      SCIPsortDownRealInt(assignments[i], sortedids[i], nlocations);
   }

   return;
}

/** choose a location to branch on, or find out that the given assignments are feasible:
 *  we choose a location for which the number of fractionally assigned medians is maximal;
 *  in case of ties, we choose the location for which the total fractional assignment value
 *  of every second median is closest to half the total fractional assignment value of all medians.
 */
static
SCIP_RETCODE chooseLocation(
   SCIP*                 scip,               /* SCIP data structure                                                    */
   SCIP_Real**           assignments,        /* matrix of location-median assignments                                  */
   int*                  location            /* pointer to store a location to branch on, or -1 in case of feasibility */
   )
{
   int nlocations;
   int nfracmedians;                         /* number of medians assigned fractionally to the current location            */
   int maxnfracmedians;                      /* maximal number of fractionally assigned medians                            */
   SCIP_Real totfrac;                        /* total fractional assignment of medians to the current location             */
   SCIP_Real halffrac;                       /* total fractional assignment of every second median to the current location */
   SCIP_Real minfracdiff;                    /* minimal difference between one half of the total fractional assignment
                                                and the total fractional assignment of every second median                 */

   int i;
   int median;

   SCIPdebugMessage("Choose a location to branch on\n");

   nlocations = SCIPprobdataGetNLocations(scip);

   /* examine each locatioon */
   *location = -1;
   maxnfracmedians = 0;
   minfracdiff = SCIPinfinity(scip);

   for( i = 0; i < nlocations; ++i )
   {
      nfracmedians = 0;
      totfrac = 0.0;
      halffrac = 0.0;

      /* ****************************************************************************************************
       * TODO: for each location, calculate
       *   * to how many medians it is assigned fractionally
       *   * the sum of all fractional assignments
       *   * the sum of all fractional assignments to an even median
       * ****************************************************************************************************
       */

      for ( median = 0; median < nlocations; ++median )
      {
         if ( !SCIPisFeasIntegral(scip, assignments[i][median] ) )
         {
           nfracmedians += 1;
           totfrac += assignments[i][median];
           if ( (median % 2) == 0 ) halffrac += assignments[i][median];
         }
      }

      SCIPdebugMessage("   -> location %d: totfrac = %g, halffrac = %g\n", i+1, totfrac, halffrac);

      if( nfracmedians > maxnfracmedians || (nfracmedians > 0 && nfracmedians == maxnfracmedians && SCIPisFeasLT(scip, ABS(halffrac - 0.5 * totfrac), minfracdiff)) )
      {
         *location = i;
         maxnfracmedians = nfracmedians;
         minfracdiff = ABS(halffrac - 0.5 * totfrac);

         SCIPdebugMessage("      -> chosen this location: maxnfracmedians = %d, minfracdiff = %g\n", maxnfracmedians, minfracdiff);
      }
   }
   return SCIP_OKAY;
}

/** branch on a location: create two child nodes and forbid assigning them to the medians alternately in the two nodes */
static
SCIP_RETCODE performBranching(
   SCIP*                 scip,               /* SCIP data structure                                  */
   int*                  sortedids,          /* array of medians sorted by fractional assignment     */
   SCIP_Real*            assignments,        /* array of median assignments                          */
   int                   location            /* the location to branch on                            */
   )
{
   SCIP_NODE* childnode;
   SCIP_CONS* childcons;
   char name[SCIP_MAXSTRLEN];

   SCIP_Bool* leftforbidden;
   SCIP_Bool* rightforbidden;
   int nlocations;

   int i;

   nlocations = SCIPprobdataGetNLocations(scip);

   SCIP_CALL( SCIPallocBufferArray(scip, &leftforbidden, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &rightforbidden, nlocations) );
   BMSclearMemoryArray(leftforbidden, nlocations);
   BMSclearMemoryArray(rightforbidden, nlocations);

   /* loop over all potential medians */
   for( i = 0; i < nlocations; ++i )
   {
      assert(!SCIPisFeasIntegral(scip, assignments[sortedids[i]]) || SCIPisFeasZero(scip, assignments[sortedids[i]]));

      /* ignore already forbidden assignments, such that the child constraints only store newly forbidden assignments;
       * otherwise, this could lead to an error when deactivating a constraint
       */
      if( SCIPpricerCpmpIsAssignmentForbidden(scip, sortedids[i], location) )
         continue;

      /* ****************************************************************************************************
       * TODO: fill the 'leftforbidden' and 'rightforbidden' arrays which describe which medians
       * cannot be assigned to the location in the left and right node, respectively;
       * note that the medians are forbidden alternately
       * ****************************************************************************************************
       */
      if ( (i % 2) == 1 )
      {
         leftforbidden[sortedids[i]] = 0;
         rightforbidden[sortedids[i]] = 1;
      }
      else
      {
         leftforbidden[sortedids[i]] = 1;
         rightforbidden[sortedids[i]] = 0;
      }
   }

   /* ****************************************************************************************************
    * TODO: create the two child nodes as well as a semiassignment constraint for each of them
    *
    * NOTE: The constraints must be explicitly added to the node and then be released.
    * ****************************************************************************************************
    */

   SCIP_CALL(SCIPcreateChild(scip, &childnode, 0, SCIPgetLocalTransEstimate(scip)));

   SCIPsnprintf(name, 24, "SemiassignConstrainsLeft");
   SCIP_CALL(SCIPcreateConsSemiassign(scip, &childcons, name, location, leftforbidden, childnode));
   SCIP_CALL(SCIPaddConsNode(scip, childnode, childcons, NULL));
   SCIP_CALL(SCIPreleaseCons(scip, &childcons));

   SCIP_CALL(SCIPcreateChild(scip, &childnode, 0, SCIPgetLocalTransEstimate(scip)));

   SCIPsnprintf(name, 25, "SemiassignConstrainsright");
   SCIP_CALL(SCIPcreateConsSemiassign(scip, &childcons, name, location, rightforbidden, childnode));
   SCIP_CALL(SCIPaddConsNode(scip, childnode, childcons, NULL));
   SCIP_CALL(SCIPreleaseCons(scip, &childcons));

   SCIPfreeBufferArray(scip, &leftforbidden);
   SCIPfreeBufferArray(scip, &rightforbidden);

   return SCIP_OKAY;
}

/**@name Callback methods
 *
 * @{
 */

/** branching execution method for fractional LP solutions */
static
SCIP_DECL_BRANCHEXECLP(branchExeclpSemiassign)
{  /*lint --e{715}*/

   int** sortedids;
   SCIP_Real** assignments;
   int location;
   int nlocations;

   int i;
   int j;

   SCIPdebugMessage("Solved LP in node %"SCIP_LONGINT_FORMAT":\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
   SCIPdebug( SCIPprintSolClusters(scip, NULL) );

   nlocations = SCIPprobdataGetNLocations(scip);

   /* allocate memory */
   SCIP_CALL( SCIPallocBufferArray(scip, &sortedids, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &assignments, nlocations) );
   for( i = 0; i < nlocations; ++i )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &sortedids[i], nlocations) );
      SCIP_CALL( SCIPallocBufferArray(scip, &assignments[i], nlocations) );
      /* initialize the array of assigned medians; this will be sorted by fractional assignment to the location */
      for( j = 0; j < nlocations; ++j )
         sortedids[i][j] = j;
   }

   SCIP_CALL( computeAssignments(scip, NULL, assignments) );
   sortMedians(scip, sortedids, assignments);

   SCIP_CALL( chooseLocation(scip, assignments, &location) );

   if( location == -1 )
      *result = SCIP_DIDNOTFIND;
   else
   {
#ifdef SCIP_DEBUG
      SCIPdebugMessage("Chosen location %d:\n", location+1);
      SCIPdebugMessage("   median ids:");
      for( j = 0; j < nlocations; ++j )
      {
         SCIPdebugPrintf(" %d", sortedids[location][j]+1);
      }
      SCIPdebugPrintf("\n");
      SCIPdebugMessage("   assignments:");
      for( j = 0; j < nlocations; ++j )
      {
         SCIPdebugPrintf(" %g", assignments[location][j]);
      }
      SCIPdebugPrintf("\n");
#endif
      //printf("asdfasdfasdf\n");
      SCIP_CALL( performBranching(scip, sortedids[location], assignments[location], location) );
      *result = SCIP_BRANCHED;
   }

   for( i = nlocations-1; i >= 0; --i )
   {
      SCIPfreeBufferArray(scip, &assignments[i]);
      SCIPfreeBufferArray(scip, &sortedids[i]);
   }
   SCIPfreeBufferArray(scip, &assignments);
   SCIPfreeBufferArray(scip, &sortedids);

   return SCIP_OKAY;
}

/**@} */

/**@name Interface methods
 *
 * @{
 */

/** creates the ryan foster branching rule and includes it in SCIP */
SCIP_RETCODE SCIPincludeBranchruleSemiassign(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_BRANCHRULEDATA* branchruledata;
   SCIP_BRANCHRULE* branchrule;

   /* create Semi assignment branching rule data */
   branchruledata = NULL;
   branchrule = NULL;
   /* include branching rule */
   SCIP_CALL( SCIPincludeBranchruleBasic(scip, &branchrule, BRANCHRULE_NAME, BRANCHRULE_DESC, BRANCHRULE_PRIORITY, BRANCHRULE_MAXDEPTH,
         BRANCHRULE_MAXBOUNDDIST, branchruledata) );
   assert(branchrule != NULL);

   SCIP_CALL( SCIPsetBranchruleExecLp(scip, branchrule, branchExeclpSemiassign) );

   return SCIP_OKAY;
}

/**@} */
