/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This NULL is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2018 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the NULL COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@NULL   dialog_cpmp.c
 * @brief  extensions to the default SCIP dialog for the capacitated p-median problem example
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "scip/dialog_default.h"
#include "dialog_cpmp.h"
#include "pub_probdata.h"
#include "pub_vardata.h"


/*
 * Callback methods of dialog
 */

/** display the raw data of the problem */
static
SCIP_DECL_DIALOGEXEC(dialogExecDisplayRawdata)
{  /*lint --e{715}*/

   /* add your dialog to history of dialogs that have been executed */
   SCIP_CALL( SCIPdialoghdlrAddHistory(dialoghdlr, dialog, NULL, FALSE) );

   if( SCIPgetStage(scip) < SCIP_STAGE_PROBLEM || SCIPgetProbData(scip) == NULL )
   {
      SCIPinfoMessage(scip, NULL, "no problem has been read yet\n");
   }
   else
   {
      SCIPprintProbData(scip);
   }

   /* next dialog will be root dialog again */
   *nextdialog = SCIPdialoghdlrGetRoot(dialoghdlr);

   return SCIP_OKAY;
}


/** display the clusters of the best primal solution */
static
SCIP_DECL_DIALOGEXEC(dialogExecDisplaySolclusters)
{  /*lint --e{715}*/
   SCIP_SOL* sol;

   /* add your dialog to history of dialogs that have been executed */
   SCIP_CALL( SCIPdialoghdlrAddHistory(dialoghdlr, dialog, NULL, FALSE) );

   sol = SCIPgetBestSol(scip);

   if( sol == NULL )
   {
      SCIPinfoMessage(scip, NULL, "no solution available\n");
   }
   else
   {
      SCIPprintSolClusters(scip, sol);
   }

   /* next dialog will be root dialog again */
   *nextdialog = SCIPdialoghdlrGetRoot(dialoghdlr);

   return SCIP_OKAY;
}



/*
 * dialog specific interface methods
 */

/** creates the cpmp dialog and includes it in SCIP */
SCIP_RETCODE SCIPincludeDialogCpmp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_DIALOG* root;
   SCIP_DIALOG* submenu;
   SCIP_DIALOG* dialog;

   /* root menu */
   root = SCIPgetRootDialog(scip);
   if( root == NULL )
   {
      SCIP_CALL( SCIPcreateRootDialog(scip, &root) );
   }

   /* display */
   if( !SCIPdialogHasEntry(root, "display") )
   {
      SCIP_CALL( SCIPincludeDialog(scip, &submenu,
         NULL,
         SCIPdialogExecMenu, NULL, NULL,
         "display", "display information", TRUE, NULL) );
      SCIP_CALL( SCIPaddDialogEntry(scip, root, submenu) );
      SCIP_CALL( SCIPreleaseDialog(scip, &submenu) );
   }
   if( SCIPdialogFindEntry(root, "display", &submenu) != 1 )
   {
      SCIPerrorMessage("display sub menu not found\n");
      return SCIP_PLUGINNOTFOUND;
   }

   /* display rawdata */
   if( !SCIPdialogHasEntry(submenu, "rawdata") )
   {
      SCIP_CALL( SCIPincludeDialog(scip, &dialog,
            NULL,
            dialogExecDisplayRawdata, NULL, NULL,
            "rawdata", "display the raw data of the problem", FALSE, NULL) );
      SCIP_CALL( SCIPaddDialogEntry(scip, submenu, dialog) );
      SCIP_CALL( SCIPreleaseDialog(scip, &dialog) );
   }

   /* display solclusters */
   if( !SCIPdialogHasEntry(submenu, "solclusters") )
   {
      SCIP_CALL( SCIPincludeDialog(scip, &dialog,
            NULL,
            dialogExecDisplaySolclusters, NULL, NULL,
            "solclusters", "display the clusters of the best primal solution", FALSE, NULL) );
      SCIP_CALL( SCIPaddDialogEntry(scip, submenu, dialog) );
      SCIP_CALL( SCIPreleaseDialog(scip, &dialog) );
   }

   return SCIP_OKAY;
}
