#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define NAME "getTMs"
#include "topo_help.h"

/* 
 * Usage: ./getTMs [-g|-b] topo outdir howmany
 *
 * topo has to be of the format specified
 * will create "howmany" tms in files "outdir"/gengrav_%d.m, "outdir"/bimodal_%d.m
 *
 * TMs is a square matrix, # of lines = # of nodes in topo
 * each line has # of fields = # of nodes, traffic from line# to col#
 * 
 */

/*
 *  Generates traffic matrices of two kinds
 *  Generalized Gravity -- The proportion of node x's traffic that goes towards another node y depends
 *                         on the popularity of node y; the more popular nodes get more traffic
 *                         True in predicting transportation volumes and traffic
 *                         Use sum of capacities of edges incident on a node to measure popularity
 *  Bimodal -- The traffic between two nodes is either "high" or "low"
 *  Scaling traffic to match utilizations:
 *       Given the above generative models, it is hard to guess what link utilizations caused by a TM.
 *       In general, we scale all TM values uniformly.
 */


#define FNAMESIZE 100
int main(int argc, char **argv){
  // topo vars
  int numnodes=-1, linknum = 0;
  char **topomatrix, *type_list;
  Link ***linkmatrix;

  // what to do -- default gravity
  int switch_gen_gravity=1, switch_gen_bimodal=0;
  
  FILE* fin;
  char *topoName, *outputDirName, getoptch;
  int num_tms;

  if(argc < 4){
    printf("Usage: %s [-g|-b] <topo> <tmdir> <howmany> \n", NAME);
    exit(0);
  }

  while( (getoptch = getopt(argc, argv, "bg")) != EOF) {
    if(DEBUG) printf("found an option, %c, optind=%d\n", getoptch, optind);
    switch( getoptch ) {
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
  if(DEBUG)
    printf("outside optind is %d, argc is %d, opt is %s\n", 
	   optind, argc, argv[optind]);

  if ( argc - optind != 3){
    printf("Usage: %s [-g|-b] <topo> <tmdir> <howmany> \n", NAME);
    exit(0);
  }
  topoName = argv[optind];
  outputDirName = argv[optind+1];
  num_tms = atoi(argv[optind+2]);

  printf("Get TMS:\n\t Topo: %s\n\t OutputDir: %s\n\t Num:%d\n", topoName, outputDirName, num_tms);
  
  fin = fopen(topoName, "r");
  if( fin == NULL ) {
    printf("Cannot open %s\n", topoName); exit(1);
  }
  numnodes = readtopo(fin, &topomatrix, &linkmatrix, &type_list, &linknum);
  if ( strlen(outputDirName) > FNAMESIZE - 15 ){
    printf("fname, overflow -- output dir name is too big\n");
    exit(343);
  }

  {
    int ind1;
    FILE* tm; 
    char fname[FNAMESIZE];
    // mkdir if needed
    sprintf(fname, "mkdir -p %s", outputDirName);
    system(fname);

    for(ind1=0; ind1<num_tms; ind1++){
      if ( switch_gen_bimodal )
	sprintf(fname, "%s/bimodal_%d.m", outputDirName, ind1+1);
      else 
	sprintf(fname, "%s/gengrav_%d.m", outputDirName, ind1+1);
      if (DEBUG) printf("generating tm %d --> file %s\n", ind1+1, fname);

      tm = fopen(fname, "w");
      if ( (tm=fopen(fname, "w")) == NULL ) 
	{ printf("cannot open tm file %s to write\n", fname); exit(343); }

      srand48(getpid()+ind1);
      if ( switch_gen_bimodal)      
	get_bimodal_tm(numnodes, linkmatrix, linknum, tm, 0);
      else       
	get_gravity_tm(numnodes, linkmatrix, linknum, tm, 0);
      fclose(tm);
    }
  }
  return 0;
}
