/* automatically generated by sed scripts from the c source named below: *//* cddlib.c: cdd library  (library version of cdd)
   written by Komei Fukuda, fukuda@math.ethz.ch
   Version 0.94i, March 9, 2018
   Standard ftp site: ftp.ifor.math.ethz.ch, Directory: pub/fukuda/cdd
*/

/* cdd : C-Implementation of the double description method for
   computing all vertices and extreme rays of the polyhedron 
   P= {x :  b - A x >= 0}.
   Please read COPYING (GNU General Public Licence) and
   the manual cddlibman.tex for detail.
*/

/*  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* The first version C0.21 was created on November 10,1993 
   with Dave Gillespie's p2c translator 
   from the Pascal program pdd.p written by Komei Fukuda. 
*/

#include "setoper.h" 
  /* set operation library header (Ver. June 1, 2000 or later) */
#include "cdd_f.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

/* Global Variables */
ddf_boolean ddf_debug               =ddf_FALSE;
ddf_boolean ddf_log                 =ddf_FALSE;
/* GLOBAL CONSTANTS and STATICS VARIABLES (to be set by ddf_set_global_constants() */
myfloat ddf_zero;
myfloat ddf_one;
myfloat ddf_purezero;
myfloat ddf_minuszero;
myfloat ddf_minusone;

time_t ddf_statStartTime; /* cddlib starting time */
long ddf_statBApivots;  /* basis finding pivots */
long ddf_statCCpivots;  /* criss-cross pivots */
long ddf_statDS1pivots; /* phase 1 pivots */
long ddf_statDS2pivots; /* phase 2 pivots */
long ddf_statACpivots;  /* anticycling (cc) pivots */
#ifdef ddf_GMPRATIONAL
long ddf_statBSpivots;  /* basis status checking pivots */
#endif
ddf_LPSolverType ddf_choiceLPSolverDefault;  /* Default LP solver Algorithm */
ddf_LPSolverType ddf_choiceRedcheckAlgorithm;  /* Redundancy Checking Algorithm */
ddf_boolean ddf_choiceLexicoPivotQ;    /* whether to use the lexicographic pivot */

/* #include <profile.h>    THINK C PROFILER */
/* #include <console.h>    THINK C PROFILER */

void ddf_DDInit(ddf_ConePtr cone)
{
  cone->Error=ddf_NoError;
  cone->CompStatus=ddf_InProgress;
  cone->RayCount = 0;
  cone->TotalRayCount = 0;
  cone->FeasibleRayCount = 0;
  cone->WeaklyFeasibleRayCount = 0;
  cone->EdgeCount=0; /* active edge count */
  cone->TotalEdgeCount=0; /* active edge count */
  ddf_SetInequalitySets(cone);
  ddf_ComputeRowOrderVector(cone);
  cone->RecomputeRowOrder=ddf_FALSE;
}

void ddf_DDMain(ddf_ConePtr cone)
{
  ddf_rowrange hh, itemp, otemp;
  ddf_boolean locallog=ddf_log; /* if ddf_log=ddf_FALSE, no log will be written.  */

  if (cone->d<=0){
    cone->Iteration=cone->m;
    cone->FeasibleRayCount=0;
    cone->CompStatus=ddf_AllFound;
    goto _L99;
  }
  if (locallog) {
     fprintf(stderr,"(Initially added rows ) = ");
     set_fwrite(stderr,cone->InitialHalfspaces);
  }
  while (cone->Iteration <= cone->m) {
    ddf_SelectNextHalfspace(cone, cone->WeaklyAddedHalfspaces, &hh);
    if (set_member(hh,cone->NonequalitySet)){  /* Skip the row hh */
      if (ddf_debug) {
        fprintf(stderr,"*The row # %3ld should be inactive and thus skipped.\n", hh);
      }
      set_addelem(cone->WeaklyAddedHalfspaces, hh);
    } else {
      if (cone->PreOrderedRun)
        ddf_AddNewHalfspace2(cone, hh);
      else{
        ddf_AddNewHalfspace1(cone, hh);
      }
      set_addelem(cone->AddedHalfspaces, hh);
      set_addelem(cone->WeaklyAddedHalfspaces, hh);
    }
    if (!cone->PreOrderedRun){
      for (itemp=1; cone->OrderVector[itemp]!=hh; itemp++);
        otemp=cone->OrderVector[cone->Iteration];
      cone->OrderVector[cone->Iteration]=hh;
        /* store the dynamic ordering in ordervec */
      cone->OrderVector[itemp]=otemp;
        /* store the dynamic ordering in ordervec */
    }
    if (locallog){
      fprintf(stderr,"(Iter, Row, #Total, #Curr, #Feas)= %5ld %5ld %9ld %6ld %6ld\n",
        cone->Iteration, hh, cone->TotalRayCount, cone->RayCount,
        cone->FeasibleRayCount);
    }
    if (cone->CompStatus==ddf_AllFound||cone->CompStatus==ddf_RegionEmpty) {
      set_addelem(cone->AddedHalfspaces, hh);
      goto _L99;
    }
    (cone->Iteration)++;
  }
  _L99:;
  if (cone->d<=0 || cone->newcol[1]==0){ /* fixing the number of output */
     cone->parent->n=cone->LinearityDim + cone->FeasibleRayCount -1;
     cone->parent->ldim=cone->LinearityDim - 1;
  } else {
    cone->parent->n=cone->LinearityDim + cone->FeasibleRayCount;
    cone->parent->ldim=cone->LinearityDim;
  }
}


void ddf_InitialDataSetup(ddf_ConePtr cone)
{
  long j, r;
  ddf_rowset ZSet;
  static ddf_Arow Vector1,Vector2;
  static ddf_colrange last_d=0;

  if (last_d < cone->d){
    if (last_d>0) {
    for (j=0; j<last_d; j++){
      ddf_clear(Vector1[j]);
      ddf_clear(Vector2[j]);
    }
    free(Vector1); free(Vector2);
    }
    Vector1=(myfloat*)calloc(cone->d,sizeof(myfloat));
    Vector2=(myfloat*)calloc(cone->d,sizeof(myfloat));
    for (j=0; j<cone->d; j++){
      ddf_init(Vector1[j]);
      ddf_init(Vector2[j]);
    }
    last_d=cone->d;
  }

  cone->RecomputeRowOrder=ddf_FALSE;
  cone->ArtificialRay = NULL;
  cone->FirstRay = NULL;
  cone->LastRay = NULL;
  set_initialize(&ZSet,cone->m);
  ddf_AddArtificialRay(cone);
  set_copy(cone->AddedHalfspaces, cone->InitialHalfspaces);
  set_copy(cone->WeaklyAddedHalfspaces, cone->InitialHalfspaces);
  ddf_UpdateRowOrderVector(cone, cone->InitialHalfspaces);
  for (r = 1; r <= cone->d; r++) {
    for (j = 0; j < cone->d; j++){
      ddf_set(Vector1[j], cone->B[j][r-1]);
      ddf_neg(Vector2[j], cone->B[j][r-1]);
    }
    ddf_Normalize(cone->d, Vector1);
    ddf_Normalize(cone->d, Vector2);
    ddf_ZeroIndexSet(cone->m, cone->d, cone->A, Vector1, ZSet);
    if (set_subset(cone->EqualitySet, ZSet)){
      if (ddf_debug) {
        fprintf(stderr,"add an initial ray with zero set:");
        set_fwrite(stderr,ZSet);
      }
      ddf_AddRay(cone, Vector1);
      if (cone->InitialRayIndex[r]==0) {
        ddf_AddRay(cone, Vector2);
        if (ddf_debug) {
          fprintf(stderr,"and add its negative also.\n");
        }
      }
    }
  }
  ddf_CreateInitialEdges(cone);
  cone->Iteration = cone->d + 1;
  if (cone->Iteration > cone->m) cone->CompStatus=ddf_AllFound; /* 0.94b  */
  set_free(ZSet);
}

ddf_boolean ddf_CheckEmptiness(ddf_PolyhedraPtr poly, ddf_ErrorType *err)
{
  ddf_rowset R, S;
  ddf_MatrixPtr M=NULL;
  ddf_boolean answer=ddf_FALSE;

  *err=ddf_NoError;

  if (poly->representation==ddf_Inequality){
	M=ddf_CopyInequalities(poly);
	set_initialize(&R, M->rowsize);
	set_initialize(&S, M->rowsize);
	if (!ddf_ExistsRestrictedFace(M, R, S, err)){
	  poly->child->CompStatus=ddf_AllFound;
	  poly->IsEmpty=ddf_TRUE;
	  poly->n=0;
	  answer=ddf_TRUE;
	}
	set_free(R);
	set_free(S);
	ddf_FreeMatrix(M);
  } else if (poly->representation==ddf_Generator && poly->m<=0){
	*err=ddf_EmptyVrepresentation;
	poly->IsEmpty=ddf_TRUE;
	poly->child->CompStatus=ddf_AllFound;
	answer=ddf_TRUE;
	poly->child->Error=*err;  
  }
  
  return answer;
}


ddf_boolean ddf_DoubleDescription(ddf_PolyhedraPtr poly, ddf_ErrorType *err)
{
  ddf_ConePtr cone=NULL;
  ddf_boolean found=ddf_FALSE;

  *err=ddf_NoError;

  if (poly!=NULL && (poly->child==NULL || poly->child->CompStatus!=ddf_AllFound)){
    cone=ddf_ConeDataLoad(poly);
    /* create a cone associated with poly by homogenization */
    time(&cone->starttime);
    ddf_DDInit(cone);
    if (poly->representation==ddf_Generator && poly->m<=0){
       *err=ddf_EmptyVrepresentation;
       cone->Error=*err;
	   goto _L99;
	}
	/* Check emptiness of the polyhedron */
	ddf_CheckEmptiness(poly,err);

    if (cone->CompStatus!=ddf_AllFound){
	  ddf_FindInitialRays(cone, &found);
	  if (found) {
	    ddf_InitialDataSetup(cone);
	    if (cone->CompStatus==ddf_AllFound) goto _L99;
	    ddf_DDMain(cone);
	    if (cone->FeasibleRayCount!=cone->RayCount) *err=ddf_NumericallyInconsistent; /* cddlib-093d */
	  }
	}
    time(&cone->endtime);
  }
    
_L99: ;

  return found;
}

ddf_boolean ddf_DoubleDescription2(ddf_PolyhedraPtr poly, ddf_RowOrderType horder, ddf_ErrorType *err)
{
  ddf_ConePtr cone=NULL;
  ddf_boolean found=ddf_FALSE;

  *err=ddf_NoError;

  if (poly!=NULL && (poly->child==NULL || poly->child->CompStatus!=ddf_AllFound)){
    cone=ddf_ConeDataLoad(poly);
    /* create a cone associated with poly by homogenization */
	cone->HalfspaceOrder=horder;  /* set the row order */
    time(&cone->starttime);
    ddf_DDInit(cone);
    if (poly->representation==ddf_Generator && poly->m<=0){
       *err=ddf_EmptyVrepresentation;
       cone->Error=*err;
	   goto _L99;
	}
	/* Check emptiness of the polyhedron */
	ddf_CheckEmptiness(poly,err);

    if (cone->CompStatus!=ddf_AllFound){
	  ddf_FindInitialRays(cone, &found);
	  if (found) {
	    ddf_InitialDataSetup(cone);
	    if (cone->CompStatus==ddf_AllFound) goto _L99;
	    ddf_DDMain(cone);
	    if (cone->FeasibleRayCount!=cone->RayCount) *err=ddf_NumericallyInconsistent; /* cddlib-093d */
	  }
	}
    time(&cone->endtime);
  }
    
_L99: ;

  return found;
}

ddf_boolean ddf_DDInputAppend(ddf_PolyhedraPtr *poly, ddf_MatrixPtr M,
  ddf_ErrorType *err)
{
  /* This is imcomplete.  It simply solves the problem from scratch.  */
  ddf_boolean found;
  ddf_ErrorType error;

  if ((*poly)->child!=NULL) ddf_FreeDDMemory(*poly);
  ddf_AppendMatrix2Poly(poly, M);
  (*poly)->representation=ddf_Inequality;
  found=ddf_DoubleDescription(*poly, &error);
  *err=error;
  return found;
}

ddf_boolean ddf_DDFile2File(char *ifile, char *ofile, ddf_ErrorType *err)
{
  /* The representation conversion from an input file to an outfile.  */
  /* modified by D. Avis to allow stdin/stdout */
  ddf_boolean found=ddf_TRUE;
  FILE *reading=NULL,*writing=NULL;
  ddf_PolyhedraPtr poly;
  ddf_MatrixPtr M, A, G;

  if (strcmp(ifile,"**stdin") == 0 )
     reading = stdin;
  else if ( ( reading = fopen(ifile, "r") )!= NULL) {
    fprintf(stderr,"input file %s is open\n", ifile);
   }
  else{
    fprintf(stderr,"The input file %s not found\n",ifile);
    found=ddf_FALSE;
    *err=ddf_IFileNotFound;
    goto _L99;
  }

  if (found){
    if (strcmp(ofile,"**stdout") == 0 )
      writing = stdout;
    else if ( (writing = fopen(ofile, "w") ) != NULL){
      fprintf(stderr,"output file %s is open\n",ofile);
      found=ddf_TRUE;
    } else {
      fprintf(stderr,"The output file %s cannot be opened\n",ofile);
      found=ddf_FALSE;
      *err=ddf_OFileNotOpen;
      goto _L99;
    }
  }

  M=ddf_PolyFile2Matrix(reading, err);
  if (*err!=ddf_NoError){
    goto _L99;
  } poly=ddf_DDMatrix2Poly(M, err);  /* compute the second representation */ ddf_FreeMatrix(M);

  if (*err==ddf_NoError) {
    ddf_WriteRunningMode(writing, poly);
    A=ddf_CopyInequalities(poly);
    G=ddf_CopyGenerators(poly);

    if (poly->representation==ddf_Inequality) {
      ddf_WriteMatrix(writing,G);
     } else {
      ddf_WriteMatrix(writing,A);
    }

    ddf_FreePolyhedra(poly);
    ddf_FreeMatrix(A);
    ddf_FreeMatrix(G);
  } 

_L99: ;
  if (*err!=ddf_NoError) ddf_WriteErrorMessages(stderr,*err);
  if (reading!=NULL) fclose(reading);
  if (writing!=NULL) fclose(writing);
  return found;
}

/* end of cddlib.c */
