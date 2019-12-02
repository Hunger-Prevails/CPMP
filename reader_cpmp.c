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

/**@file   reader_cpmp.c
 * @brief  file reader for capacitated p-median problems
 * @author Christian Puchert
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include <stdio.h>

#include "reader_cpmp.h"
#include "probdata.h"


#define READER_NAME             "cpmp"
#define READER_DESC             "file reader for capacitated p-median problems"
#define READER_EXTENSION        "cpmp"


/*
 * Callback methods of reader
 */


/** problem reading method of reader */
static
SCIP_DECL_READERREAD(readerReadCpmp)
{  /*lint --e{715}*/
   SCIP_FILE* file;
   int nlines;
   SCIP_Bool readerror;

   char buffer[SCIP_MAXSTRLEN];              /* the current line in the input file */
   int i;

   int nlocations;
   int nclusters;
   SCIP_Longint** distances;
   SCIP_Longint* demands;
   SCIP_Longint* capacities;

   *result = SCIP_DIDNOTRUN;

   /* open file */
   file = SCIPfopen(filename, "r");
   if( file == NULL )
   {
      SCIPerrorMessage("cannot open file <%s> for reading\n", filename);
      SCIPprintSysError(filename);
      return SCIP_NOFILE;
   }

   //printf("%s", "asdfsdf");
   //SCIPerrorMessage("cannot open file <%s> for reading\n", filename);

   nlines = 0;
   readerror = FALSE;

   /* read numbers of locations and clusters */
   if( !SCIPfeof(file) )
   {
      int ntokens;

      /* get next nlines */
      if( SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL )
         readerror = TRUE;

      ntokens = sscanf(buffer, "%d %d", &nlocations, &nclusters);

      if( ntokens < 2 )
      {
         SCIPwarningMessage(scip, "invalid input line %d in file <%s>: <%s>\n", nlines, filename, buffer);
         SCIPwarningMessage(scip, "only %d entries found, need 2.\n", ntokens);
         readerror = TRUE;
      }

      ++nlines;
   }

   //printf("%d %d", nlocations, nclusters);

   /* allocate memory for the demand and capacity vectors as well as the distance matrix */
   SCIP_CALL( SCIPallocBufferArray(scip, &demands, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &capacities, nlocations) );
   SCIP_CALL( SCIPallocBufferArray(scip, &distances, nlocations) );
   for( i = 0; i < nlocations; ++i )
   {
      SCIP_CALL( SCIPallocBufferArray(scip, &distances[i], nlocations) );
   }

   /* ********************************************************************************
    * TODO: read in the distance matrix; complete the 'while' loop first
    * by replacing the 'TRUE'.
    * How  many lines do you need to read in?
    * ********************************************************************************
    */
   /* read the distance matrix */
   while( !SCIPfeof(file) && !readerror && nlines <= nlocations )
   {
      char* pos;                             /* current position in the input line */
      char* next;                            /* next position in the input line    */
      SCIP_Longint entry;                    /* current entry                      */
      int nentries;                          /* number of entries read to far      */

      /* ********************************************************************************
       * TODO: read the current line on the distance matrix;
       * each line contains nlocations many entries
       * ********************************************************************************
       */

      if ( SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL )
    	  readerror = TRUE;

      //printf("%s", buffer);

      for ( pos = buffer, nentries = 0; nentries < nlocations; pos = next, ++nentries )
      {
    	  entry = strtol(pos, &next, 10);
    	  //printf("%s", next);
    	  if( next != pos )
    		  distances[nlines - 1][nentries] = entry;
    	  else
    		  break;
      }

      ++nlines;

      if( nentries < nlocations )
      {
         SCIPwarningMessage(scip, "invalid input line %d in file <%s>: <%s>\n", nlines, filename, buffer);
         SCIPwarningMessage(scip, "too few distance entries.\n");
         readerror = TRUE;
         break;
      }
   }

   /*for (int i = 0; i < nlocations; ++i)
   {
	   for (int j = 0; j < nlocations; ++j)
	   {
		   printf("%lld ", distances[i][j]);
	   }
	   printf("\n");
   }*/

   if( nlines < nlocations + 1 )
   {
      SCIPwarningMessage(scip, "invalid input in file <%s>, distance matrix has only %d rows (%d needed).\n", filename, nlines-1, nlocations);
      readerror = TRUE;
   }

   /* read the demands */
   if( !SCIPfeof(file) && !readerror )
   {
      char* pos;                             /* current position in the input line */
      char* next;                            /* next position in the input line    */
      SCIP_Longint entry;                    /* current entry                      */
      int nentries;                          /* number of entries read to far      */

      /* get next line */
      if( SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL )
         readerror = TRUE;

      /* go through all entries in this line */
      for( pos = buffer, nentries = 0; nentries < nlocations; pos = next, ++nentries )
      {
         entry = strtol(pos, &next, 10);

         if( next != pos )
            demands[nentries] = entry;
         else
            break;
      }

      ++nlines;

      if( nentries < nlocations )
      {
         SCIPwarningMessage(scip, "invalid input line %d in file <%s>: <%s>\n", nlines, filename, buffer);
         SCIPwarningMessage(scip, "too few demand entries.\n");
         readerror = TRUE;
      }
   }
   else
      readerror = TRUE;

   /* read the capacities */
   if( !SCIPfeof(file) && !readerror )
   {
      char* pos;                             /* current position in the input line */
      char* next;                            /* next position in the input line    */
      SCIP_Longint entry;                    /* current entry                      */
      int nentries;                          /* number of entries read to far      */

      /* get next line */
      if( SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL )
         readerror = TRUE;

      /* go through all entries in this line */
      for( pos = buffer, nentries = 0; nentries < nlocations; pos = next, ++nentries )
      {
         entry = strtol(pos, &next, 10);

         if( next != pos )
            capacities[nentries] = entry;
         else
            break;
      }

      ++nlines;

      if( nentries < nlocations )
      {
         SCIPwarningMessage(scip, "invalid input line %d in file <%s>: <%s>\n", nlines, filename, buffer);
         SCIPwarningMessage(scip, "too few capacity entries.\n");
         readerror = TRUE;
      }
   }
   else
      readerror = TRUE;

   /* If reading was successful, create the problem and save the data */
   if( !readerror )
   {
      SCIP_CALL( SCIPcreateProbBasic(scip, filename) );
      SCIP_CALL( SCIPcreateProbCpmp(scip, nlocations, nclusters, distances, demands, capacities) );
   }
   //printf("asdfasdf");
   /* free memory */
   for( i = 0; i < nlocations; ++i )
   {
      SCIPfreeBufferArray(scip, &distances[i]);
   }
   SCIPfreeBufferArray(scip, &distances);
   SCIPfreeBufferArray(scip, &capacities);
   SCIPfreeBufferArray(scip, &demands);

   (void) SCIPfclose(file);

   if( readerror )
      return SCIP_READERROR;

   *result = SCIP_SUCCESS;
   //printf("asdfasdf");
   return SCIP_OKAY;
}


/*
 * reader specific interface methods
 */

/** includes the cpmp file reader in SCIP */
SCIP_RETCODE SCIPincludeReaderCpmp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_READER* reader = NULL;

   /* include reader */
   SCIP_CALL( SCIPincludeReaderBasic(scip, &reader, READER_NAME, READER_DESC, READER_EXTENSION, NULL) );
   assert(reader != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetReaderRead(scip, reader, readerReadCpmp) );
   //printf("asdfasdf");
   return SCIP_OKAY;
}
