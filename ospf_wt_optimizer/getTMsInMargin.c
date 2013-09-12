#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define NAME "getTMsInMargin"
#include "topo_help.h"

/* 
 * Usage: ./getTMsInMargin [-g|-b] topo tmdir whichTM margin howmany
 *
 * topo is a generic topo file
 * tmdir is a name like --- directory/
 * whichTM is a number say 1 to mean gengrav_1 or bimodal_1 in that directory
 * margin is a float >= 1.0 say 1.1, 2.0 
 * howmany sample TMs to create off the base TM in that margin range?
 *
 */

/*
 *  Given a base traffic matrix, generates "howmany" TMs that are off from the 
 *  base by "margin"
 *
 *  Each entry in the TM increases or decreases with prob 0.5
 *    The degree of perturbation is a multiply factor 
 *    chosen uniformly in the ranges [1/margin, 1] and [1, margin]
 */


#define OUTDIRSIZE 300
int main(int argc, char **argv){
  FILE* fin;
  int numnodes,linknum;
  char **topomatrix, *type_list;
  Link*** linkmatrix;
  int switch_gen_gravity=1, switch_gen_bimodal=0;

  if(argc < 6){
    printf("Usage: %s [-g|-b] <topo> <tmdir> <whichTM> <margin> <howmany>\n", NAME);
    exit(0);
  }
  
  char getoptch;
  while( (getoptch = getopt(argc, argv, "bg")) != EOF) {
    if (DEBUG) printf("found an option, %c, optind=%d\n", getoptch, optind);
    switch ( getoptch ) {
    case 'g':
      switch_gen_gravity = 1;
      break;
    case 'b':
      switch_gen_bimodal = 1;
      break;
    default:
      printf ("Unknown option\n");
    }
  }
  if (DEBUG)
    printf("outside optind is %d, argc is %d, opt is %s\n", 
	   optind, argc, argv[optind]);

  if(argc - optind != 5){
    printf("Usage: %s [-g|-b] <topo> <tmdir> <whichTM> <margin> <howmany>\n", NAME);
    exit(0);
  }


  char *topoFile = argv[optind],
    *dirname = argv[optind+1],
    *marginStr= argv[optind+3];
  int which = atoi(argv[optind+2]), 
    howmany = atoi(argv[optind+4]);
  double margin = atof(marginStr);


  printf("getTMsInMargin: \n\tTopo %s\n\tGenerating %d TMS\n\tMargin %g\n\tFrom %s/%s_%d.m\n",
	 topoFile, howmany, margin, dirname,
	 switch_gen_bimodal?"bimodal":"gengrav",
	 which);
  if(margin < 1.0) { printf("margin %g has to be >1\n", margin); exit(2); }

  // open topo
  fin = fopen(topoFile, "r");
  if( fin == NULL ) {
    printf("Cannot open %s\n", topoFile); exit(1);
  }
  numnodes = readtopo(fin, &topomatrix, &linkmatrix, &type_list, &linknum);

  
  // read tm
  double **demands = get2ddouble(numnodes);
  {
    char fileName[1000];
    sprintf(fileName, "%s/%s_%d.m", dirname, switch_gen_bimodal?"bimodal":"gengrav", which);
    read2dArray_file(numnodes, fileName, demands);
  }


  int ind1;
  FILE* fout;
  char cmd[OUTDIRSIZE+30];
  char outdir[OUTDIRSIZE];
  if ( snprintf(outdir, OUTDIRSIZE, "%s/%s_%d_marginTMs/margin_%s/", 
		dirname, switch_gen_bimodal?"bimodal":"gengrav",
		which, marginStr) >= OUTDIRSIZE ){
    printf("overflow in dirname\n"); exit(343);
  }

  // make the output dir
  sprintf(cmd, "mkdir -p %s", outdir);
  system(cmd);


  double **tempdemands  = get2ddouble(numnodes);
  for(ind1=0; ind1<howmany; ind1++){
    // generate a TM off by a margin
    getTMinmargin(numnodes, demands, margin, &tempdemands);

    // write it out
    sprintf(cmd, "%s/%sM_%d.m", outdir, 
	    switch_gen_bimodal?"bimodal":"gengrav", ind1+1);
    fout = fopen(cmd, "w");
    if(fout == NULL) { printf("cannot open %s\n", cmd); exit(3); }
    if (DEBUG) printf("seed is %d\n", getpid()+ind1);
    srand48(getpid()+ind1);
    __writeweights(numnodes, tempdemands, fout, WRITETM);
    fclose(fout);
  }
}
