#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <float.h>

#define NAME "getOptOSPFWeights"
#include "topo_help.h"

/* 
 * Usage: ./getOptOSPFWeights [-m|-v] topo tmdir fileTag numTMs [outdir]
 *
 *  Given a topology and a certain number of TMs finds optimal weights.
 *
 * topo is a generic topo file
 * tmdir is location of tms
 * optimize over TMs in tmdir/fileTag_%d    for 1, ..., numTMs
 * output goes into outdir if present, else currentDirector
 * outputfile will be tagged 
 *             fileTag_%d.weights for indiv
 *             fileTag_%d_%d.weights for multiple TMs at the same time
 *             fileTag_%d_%d.fail_weights for indiv, also optimize over failures
 * use -m to yield a single weight-set over all TMs
 * use -v to simultaneously optimize over failures
 *
 */

#define OUTDIRSIZE 300
int main(int argc, char **argv){
  FILE* fin, *fout;
  int numnodes,linknum;
  char **topomatrix, *type_list, getoptch;
  Link*** linkmatrix;
  int ** path_specification_matrix;
  double ** path_preference_matrix;
  int switch_opt_one_at_a_time=1, switch_optimize_over_fails=0;
  double cost_threshold = 0;  
  char *given_weights_filename=0;
  char *path_specifier_filename=0;
  char *path_preference_filename=0;


 

  /*if(argc < 5){
    printf("Usage: %s [-m|-v] <topo> <tmdir> <fileTag> <numTMs> [OUTDIR]\n", NAME);
    exit(0);
  }*/
  
  while( (getoptch = getopt(argc, argv, "mv")) != EOF) {
    if(DEBUG) printf("found an option, %c, optind=%d\n", getoptch, optind);
    switch( getoptch ) {
    case 'm':
      switch_opt_one_at_a_time = 0;
      break;
    case 'v':
      switch_optimize_over_fails = 1;
      break;
    default:
      printf ("Unknown option, ignore\n");
    }
  }

 // if (DEBUG)
    printf("outside optind is %d, argc is %d, opt is %s\n", optind, argc, argv[optind]);

/*  if ( argc-optind < 4 || argc-optind > 5){
    printf("Usage: %s [-i|-v] <topo> <tmdir> <fileTag> <numTMs> [OUTDIR]\n", NAME);
    exit(0);
  }*/

  char *topoFile = argv[optind], *tmDir = argv[optind+1], *fileTag = argv[optind+2];
  int how_many_tms = atoi(argv[optind+3]);
  if ( how_many_tms < 1 ) {
    printf("invalid # of tms %d\n", how_many_tms); exit(437);
  }

if(argc>optind+4)
{
printf("I am in here for the time being\n");
Path_numberinS=atoi(argv[optind+4]);
given_weights_filename=argv[optind+5];
path_specifier_filename=argv[optind+6];
path_preference_filename=argv[optind+7];
cost_threshold=atof(argv[optind+8]);
bestcostfile=argv[optind+9];
append=argv[optind+10];
}

if(Path_numberinS==0)
{
	cost_threshold=0;
}
printf("I got here\n");
  char outdir [OUTDIRSIZE];
  if ( argc - optind == 5 ){
    if ( strlen(argv[optind+4]) > OUTDIRSIZE - 1 )
      { printf("too big outdirname\n"); exit(436); }
    strcpy(outdir, argv[optind+4]);
  }else
    strcpy(outdir, ".");
  
  printf("getOptOSPFWeights: \n\tTopo %s\n\tTMs from Dir %s\n\tFileTag %s\n\tTMs 1-%d\n\tOutputDir %s\n\tFlags: One-by-one %s, Optimize-Fails %s\n",
	 topoFile, tmDir, fileTag, how_many_tms, outdir, 
	 switch_opt_one_at_a_time?"Yes":"No",
	 switch_optimize_over_fails?"Yes":"No" );

  if(switch_optimize_over_fails){
    if (how_many_tms > 1 && !switch_opt_one_at_a_time)
      printf("\n\tOptimizing over failures not built for multiple TMS; Ignore optimize_over_fails\n");
    else    printf("\n\tOptimizing over 1 failures\n");
  }
  if(how_many_tms > 1 && switch_opt_one_at_a_time)
    printf("\n\tOptimizing over singleton TMs\n");
  else printf("\n\tOptimizing over multiple TMs\n");

  // open topo
  fin = fopen(topoFile, "r");
  if( fin == NULL ) { printf("Cannot open %s\n", topoFile); exit(1); }
  numnodes = readtopo(fin, &topomatrix, &linkmatrix, &type_list, &linknum);

//printf("********** PLEASE PRINT THE NUMBER OF PATH PREFERENCES SPECIFIED ***********\n");
//scanf("%d",&Path_numberinS);
//printf("********** PLEASE PRINT THE FILENAME FOR THE GIVEN WEIGHTS(E.G../gengrav_1_4.weights ********\n");
//scanf("%s",given_weights_filename);
//printf("********** PLEASE ENTER THE PATH SPECIFIER FILE NAME(E.G. ./pathspecifier\n***********");
//scanf("%s",path_specifier_filename);
//printf("********** AND THE PATH PREFERENCE FILNAME?(E.G. ./pathpref\n");
//scanf("%s",path_preference_filename);

if(Path_numberinS>0)
{

path_specification_matrix=(int **)malloc(sizeof(int *)*Path_numberinS);
path_preference_matrix=(double **)malloc(sizeof(double *)*Path_numberinS);
  int i;
  for(i=0; i<Path_numberinS; i++)
  {
	path_specification_matrix[i]=(int *)malloc(sizeof(int *)*numnodes);
	path_preference_matrix[i]=(double *)malloc(sizeof(double)*Path_numberinS);
  }
/*Here we read the links that constitute the path in the form of a k*n matrix where k is the number of path in S. 
Format: each row contains a list of numbers where each number belongs to the interval [0,n]. If the number of nodes involved in a path are less than n then the remaining elements of the matrix are set to -1.*/

InitPathSpecMatrix(numnodes,path_specification_matrix, path_specifier_filename);
read2dArray_file(Path_numberinS, path_preference_filename, path_preference_matrix);


}



  // outFilePrefix
  char outFilePrefix[OUTDIRSIZE+100];
  sprintf(outFilePrefix, "%s/%s", outdir, fileTag);

  /* Load given weights */
  double** given_weights = 0;
  int ind1;
  if (given_weights_filename)
  {
       given_weights = (double**)malloc(sizeof(double*) * numnodes);
    for (ind1 = 0; ind1 < numnodes; ++ind1)
    {
      given_weights[ind1] = (double*)malloc(sizeof(double) * numnodes); 
    }
printf("%s\n",given_weights_filename);
    read2dArray_file(numnodes, given_weights_filename, given_weights);

  }

  {
    double **dist;
    char ***succ;
    int ind1; 
    if(switch_opt_one_at_a_time) {
      double **tm = get2ddouble(numnodes);
      for(ind1=0; ind1<how_many_tms; ind1++){
	readTMs(numnodes, tmDir, ind1+BASETM, tm, fileTag);
	
	setoptimalweights(&tm, numnodes, linknum, path_preference_matrix, path_specification_matrix, linkmatrix, topomatrix, given_weights,
			  ind1+BASETM, 1, switch_optimize_over_fails, outFilePrefix, cost_threshold);
      }
    }else {
      double ***tms = (double ***) malloc(how_many_tms * sizeof(double**));
      for(ind1=0; ind1<how_many_tms; ind1++){
	tms[ind1] = get2ddouble(numnodes);
	readTMs(numnodes, tmDir, BASETM+ind1, tms[ind1], fileTag);
      }
	printf("man alan in tu hastam dumbass\n");
      setoptimalweights(tms, numnodes, linknum, path_preference_matrix, path_specification_matrix, linkmatrix, topomatrix, given_weights,
			BASETM, how_many_tms, 0, outFilePrefix, cost_threshold);
    }
  }

  if (given_weights)
  {
    for (ind1 = 0; ind1 < numnodes; ++ind1)
    {
      free(given_weights[ind1]);
    }
    free(given_weights);

  int i;

  for(i=0; i<Path_numberinS; i++)
  {
	free(path_specification_matrix[i]);
	free(path_preference_matrix[i]);
  }
if(Path_numberinS!=0)
{
free(path_specification_matrix);
free(path_preference_matrix);
}
  }
	exit(0);
}
