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

/**@file   cons_semiassign.c
 * @brief  constraint handler for semiassignment constraints
 * @author Christian Puchert
 * @author Jonas Witt
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "cons_semiassign.h"
#include "pricer_cpmp.h"
#include "pub_probdata.h"
#include "pub_vardata.h"


/* fundamental constraint handler properties */
#define CONSHDLR_NAME          "semiassign"
#define CONSHDLR_DESC          "constraint handler for branching decisions in capacitated p-median problems"
#define CONSHDLR_ENFOPRIORITY         0 /**< priority of the constraint handler for constraint enforcing */
#define CONSHDLR_CHECKPRIORITY        0 /**< priority of the constraint handler for checking feasibility */
#define CONSHDLR_EAGERFREQ          100 /**< frequency for using all instead of only the useful constraints in separation,
                                              *   propagation and enforcement, -1 for no eager evaluations, 0 for first only */
#define CONSHDLR_NEEDSCONS         TRUE /**< should the constraint handler be skipped if no constraints are available? */

/* optional constraint handler properties */
#define CONSHDLR_PROPFREQ             1 /**< frequency for propagating domains; zero means only preprocessing propagation */
#define CONSHDLR_DELAYPROP        FALSE /**< should propagation method be delayed if other propagators found reductions? */
#define CONSHDLR_PROP_TIMING       SCIP_PROPTIMING_BEFORELP/**< propagation timing mask of the constraint handler*/


/*
 * Data structures
 */

/** constraint data for semiassign constraints */
struct SCIP_ConsData
{
   int                   location;           /* location for which certain medians are forbidden                                */
   SCIP_Bool*            forbidden;          /* for each median, the information whether the location may not be assigned to it */

   SCIP_NODE*            node;               /* node for which the constraint is valid                                                   */
   SCIP_Bool             propagate;          /* Has the constrained to be propagated? TRUE if the subtree
                                                below the node is entered and new variables have been created since the last propagation */
   int                   npropvars;          /* number of variables present in the problem the last time the constrained was propagated  */
};


/*
 * Callback methods of constraint handler
 */

/** initialization method of constraint handler (called after problem was transformed) */
#define consInitSemiassign NULL

/** frees specific constraint data */
static
SCIP_DECL_CONSDELETE(consDeleteSemiassign)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(consdata != NULL);
   assert(*consdata != NULL);

   SCIPfreeMemoryArrayNull(scip, &(*consdata)->forbidden);
   SCIPfreeMemory(scip, consdata);

   return SCIP_OKAY;
}


/** constraint enforcing method of constraint handler for LP solutions */
#define consEnfolpSemiassign NULL

/** constraint enforcing method of constraint handler for pseudo solutions */
#define consEnfopsSemiassign NULL

/** feasibility check method of constraint handler for integral solutions */
#define consCheckSemiassign NULL

/** domain propagation method of constraint handler;
 *  fix those variables to zero whose represented clusters assign a location to a forbidden median
 */
static
SCIP_DECL_CONSPROP(consPropSemiassign)
{  /*lint --e{715}*/
   SCIP_CONSDATA* consdata;
   SCIP_VAR** vars;
   int nvars;
   int nfixedvars;
   SCIP_Bool fixed;
   SCIP_Bool infeasible;

   int c;
   int i;

   SCIP_CALL( SCIPgetVarsData(scip, &vars, &nvars, NULL, NULL, NULL, NULL) );

   *result = SCIP_DIDNOTFIND;

   SCIPdebugMessage("consPropSemiassign, nconss = %d\n", nconss);

   for( c = 0; c < nconss && *result != SCIP_CUTOFF; ++c )
   {
      assert(SCIPconsIsActive(conss[c]));

      consdata = SCIPconsGetData(conss[c]);
      assert(consdata != NULL);

      if( consdata->propagate )
      {
         SCIPdebugMessage("   -> propagate constraint %s (location = %d)\n", SCIPconsGetName(conss[c]), consdata->location+1);

         nfixedvars = 0;
         for( i = consdata->npropvars; i < nvars; ++i )
         {
            if( !SCIPisFeasZero(scip, SCIPvarGetUbLocal(vars[i])) &&
               consdata->forbidden[SCIPvarGetMedian(vars[i])] && SCIPisLocationInCluster(vars[i], consdata->location) )
            {
               infeasible = FALSE;
               fixed = FALSE;

               SCIP_CALL( SCIPfixVar(scip, vars[i], 0.0, &infeasible, &fixed) );
               ++nfixedvars;
               SCIPdebug( SCIPprintVarData(scip, vars[i]) );

               if( infeasible )
               {
                  *result = SCIP_CUTOFF;
                  break;
               }
               else
               {
                  *result = SCIP_REDUCEDDOM;
                  assert(fixed);
               }
            }
         }

         SCIPdebugMessage("   -> %d variables fixed to zero.\n", nfixedvars);

         consdata->propagate = FALSE;
         consdata->npropvars = i;
      }
   }

   return SCIP_OKAY;
}


/** variable rounding lock method of constraint handler */
static
SCIP_DECL_CONSLOCK(consLockSemiassign)
{  /*lint --e{715}*/
   return SCIP_OKAY;
}


/** constraint activation notification method of constraint handler */
static
SCIP_DECL_CONSACTIVE(consActiveSemiassign)
{  /*lint --e{715}*/
   SCIP_CONSDATA* consdata;
   int nvars;

   consdata = SCIPconsGetData(cons);
   assert(consdata != NULL);

   nvars = SCIPgetNVars(scip);
   assert(consdata->npropvars <= nvars);

   SCIPdebugMessage("Activate constraint %s\n", SCIPconsGetName(cons));

   /* notify SCIP that the branching decision has to be propagated to the newly created master variables */
   if( consdata->npropvars < nvars )
   {
      SCIPdebugMessage("constraint %s needs to be propagated\n", SCIPconsGetName(cons));
      consdata->propagate = TRUE;
      SCIP_CALL( SCIPrepropagateNode(scip, consdata->node) );
   }

   /* notify the pricer about the forbidden assignments */
   SCIPpricerCpmpForbidAssignments(scip, consdata->location, consdata->forbidden);

   return SCIP_OKAY;
}


/** constraint deactivation notification method of constraint handler */
static
SCIP_DECL_CONSDEACTIVE(consDeactiveSemiassign)
{  /*lint --e{715}*/
   SCIP_CONSDATA* consdata;

   consdata = SCIPconsGetData(cons);
   assert(consdata != NULL);

   SCIPdebugMessage("Deactivate constraint %s\n", SCIPconsGetName(cons));

   SCIPpricerCpmpAllowAssignments(scip, consdata->location, consdata->forbidden);

   consdata->propagate = FALSE;

   return SCIP_OKAY;
}


/** constraint display method of constraint handler */
static
SCIP_DECL_CONSPRINT(consPrintSemiassign)
{  /*lint --e{715}*/
   SCIP_CONSDATA* consdata;
   int nlocations;

   int i;

   consdata = SCIPconsGetData(cons);
   assert(consdata != NULL);

   nlocations = SCIPprobdataGetNLocations(scip);

   SCIPinfoMessage(scip, file, "\n");
   SCIPinfoMessage(scip, file, "   Location: %d\n", consdata->location+1);
   SCIPinfoMessage(scip, file, "   Forbidden medians:");
   for( i = 0; i < nlocations; ++i )
   {
      if( consdata->forbidden[i] )
      {
         SCIPinfoMessage(scip, file, " %d", i+1);
      }
   }
   SCIPinfoMessage(scip, file, "\n");

   return SCIP_OKAY;
}


/*
 * constraint specific interface methods
 */

/** creates the handler for semiassign constraints and includes it in SCIP */
SCIP_RETCODE SCIPincludeConshdlrSemiassign(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CONSHDLR* conshdlr;

   conshdlr = NULL;

   /* include constraint handler */
   SCIP_CALL( SCIPincludeConshdlrBasic(scip, &conshdlr, CONSHDLR_NAME, CONSHDLR_DESC,
         CONSHDLR_ENFOPRIORITY, CONSHDLR_CHECKPRIORITY, CONSHDLR_EAGERFREQ, CONSHDLR_NEEDSCONS,
         consEnfolpSemiassign, consEnfopsSemiassign, consCheckSemiassign, consLockSemiassign,
         NULL) );
   assert(conshdlr != NULL);

   /* set non-fundamental callbacks via specific setter functions */
   SCIP_CALL( SCIPsetConshdlrActive(scip, conshdlr, consActiveSemiassign) );
   SCIP_CALL( SCIPsetConshdlrDeactive(scip, conshdlr, consDeactiveSemiassign) );
   SCIP_CALL( SCIPsetConshdlrDelete(scip, conshdlr, consDeleteSemiassign) );
   SCIP_CALL( SCIPsetConshdlrInit(scip, conshdlr, consInitSemiassign) );
   SCIP_CALL( SCIPsetConshdlrPrint(scip, conshdlr, consPrintSemiassign) );
   SCIP_CALL( SCIPsetConshdlrProp(scip, conshdlr, consPropSemiassign, CONSHDLR_PROPFREQ, CONSHDLR_DELAYPROP,
         CONSHDLR_PROP_TIMING) );

   return SCIP_OKAY;
}

/** creates and captures a semiassign constraint
 *
 *  @note the constraint gets captured, hence at one point you have to release it using the method SCIPreleaseCons()
 */
SCIP_RETCODE SCIPcreateConsSemiassign(
   SCIP*                 scip,               /**< SCIP data structure                                                             */
   SCIP_CONS**           cons,               /**< pointer to hold the created constraint                                          */
   const char*           name,               /**< name of constraint                                                              */
   int                   location,           /**< location for which certain medians are forbidden                                */
   SCIP_Bool*            forbidden,          /**< for each median, the information whether the location may not be assigned to it */
   SCIP_NODE*            node                /**< node for which the constraint is valid                                          */
   )
{
   SCIP_CONSHDLR* conshdlr;
   SCIP_CONSDATA* consdata;

   /* find the semiassign constraint handler */
   conshdlr = SCIPfindConshdlr(scip, CONSHDLR_NAME);
   if( conshdlr == NULL )
   {
      SCIPerrorMessage("semiassign constraint handler not found\n");
      return SCIP_PLUGINNOTFOUND;
   }

   /* create constraint data */
   consdata = NULL;
   SCIP_CALL( SCIPallocMemory(scip, &consdata) );
   assert(consdata != NULL);

   consdata->location = location;
   SCIP_CALL( SCIPduplicateMemoryArray(scip, &consdata->forbidden, forbidden, SCIPprobdataGetNLocations(scip)) );
   consdata->node = node;
   consdata->propagate = TRUE;
   consdata->npropvars = 0;

   /* create constraint */
   SCIP_CALL( SCIPcreateCons(scip, cons, name, conshdlr, consdata, FALSE, FALSE, TRUE, TRUE, TRUE,
         TRUE, FALSE, FALSE, FALSE, TRUE) );

   return SCIP_OKAY;
}
