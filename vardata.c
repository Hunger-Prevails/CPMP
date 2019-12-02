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

/**@file   vardata.c
 * @brief  methods for handling capacitated p-median problem variable data
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "vardata.h"
#include "pub_vardata.h"
#include "struct_vardata.h"


/** frees user data of transformed variable (called when the transformed variable is freed) */
static
SCIP_DECL_VARDELTRANS(freeVarData)
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(vardata != NULL);

   SCIPfreeMemoryArray(scip, &(*vardata)->locations);
   SCIPfreeMemory(scip, vardata);

   return SCIP_OKAY;
}


/** create variable data */
SCIP_RETCODE SCIPcreateVarData(
   SCIP*                 scip,
   SCIP_VAR*             var,
   int                   median,
   int*                  locations,
   int                   nlocations
   )
{
   SCIP_VARDATA* vardata;

   assert(scip != NULL);

   /* allocate memory */
   vardata = NULL;
   SCIP_CALL( SCIPallocMemory(scip, &vardata) );
   assert(vardata != NULL);

   /* copy variable data */
   vardata->median = median;
   vardata->nlocations = nlocations;
   SCIP_CALL( SCIPduplicateMemoryArray(scip, &vardata->locations, locations, nlocations) );

   /* add the variable data to the variable and set the destructor */
   SCIPvarSetData(var, vardata);
   SCIPvarSetDeltransData(var, freeVarData);

   return SCIP_OKAY;
}


/** print the variable data */
void SCIPprintVarData(
   SCIP*                 scip,
   SCIP_VAR*             var
   )
{
   SCIP_VARDATA* vardata;
   int i;

   assert(var != NULL);

   vardata = SCIPvarGetData(var);
   assert(vardata != NULL);

   SCIPinfoMessage(scip, NULL, "Variable: %s\n", SCIPvarGetName(var));
   SCIPinfoMessage(scip, NULL, "   Median: %d\n", vardata->median+1);
   SCIPinfoMessage(scip, NULL, "   Locations:");
   for( i = 0; i < vardata->nlocations; ++i )
   {
      SCIPinfoMessage(scip, NULL, " %d", vardata->locations[i]+1);
   }
   SCIPinfoMessage(scip, NULL, "\n", vardata->median);

   return;
}

/** for a given solution, print the represented clusters */
void SCIPprintSolClusters(
   SCIP*                 scip,
   SCIP_SOL*             sol
   )
{
   SCIP_VAR** vars;
   int nvars;
   int v;

   vars = SCIPgetVars(scip);
   nvars = SCIPgetNVars(scip);

   for( v = 0; v < nvars; ++v )
   {
      SCIP_Real solval;

      solval = SCIPgetSolVal(scip, sol, vars[v]);
      assert(!SCIPisFeasNegative(scip, solval));

      if( SCIPisFeasPositive(scip, solval) )
      {
         SCIPinfoMessage(scip, NULL, "%-32s", SCIPvarGetName(vars[v]));
         if( solval == SCIP_UNKNOWN ) /*lint !e777*/
            SCIPinfoMessage(scip, NULL, "              unknown");
         else if( SCIPisInfinity(scip, solval) )
            SCIPinfoMessage(scip, NULL, "            +infinity");
         else
            SCIPinfoMessage(scip, NULL, " %20.15g", solval);
         SCIPinfoMessage(scip, NULL, " \t(obj:%.15g)\n", SCIPvarGetUnchangedObj(vars[v]));

         SCIPprintVarData(scip, vars[v]);
      }
   }

   return;
}

/** get the median the variable belongs to */
int SCIPvarGetMedian(
   SCIP_VAR*             var
   )
{
   SCIP_VARDATA* vardata;

   assert(var != NULL);

   vardata = SCIPvarGetData(var);
   assert(vardata != NULL);

   return vardata->median;
}

/** get the locations covered by the cluster represented by the variable */
int* SCIPvarGetLocations(
   SCIP_VAR*             var
   )
{
   SCIP_VARDATA* vardata;

   assert(var != NULL);

   vardata = SCIPvarGetData(var);
   assert(vardata != NULL);

   return vardata->locations;
}

/** get the number of locations covered by the cluster represented by the variable */
int SCIPvarGetNLocations(
   SCIP_VAR*             var
   )
{
   SCIP_VARDATA* vardata;

   assert(var != NULL);

   vardata = SCIPvarGetData(var);
   assert(vardata != NULL);

   return vardata->nlocations;
}

/** check if a given location is covered by the cluster represented by the variable */
SCIP_Bool SCIPisLocationInCluster(
   SCIP_VAR*             var,
   int                   location
   )
{
   SCIP_VARDATA* vardata;
   int i;

   assert(var != NULL);

   vardata = SCIPvarGetData(var);
   assert(vardata != NULL);

   for( i = 0; i < vardata->nlocations; ++i)
      if( vardata->locations[i] == location )
         return TRUE;
   return FALSE;
}
