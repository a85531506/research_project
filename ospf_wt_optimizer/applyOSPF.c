#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define NAME "applyospf"
#include "topo_help.h"

/* 
 * Usage: ./applyospf  topo weights/TM [fail string]
 *
 * weights is of the form --- directory num ... directory/opt_g_weights_{num}
 * TM is just the previous number --- directory/gengrav_%d.m
 * fail string is of the form "a b c"
 */
int main(int argc, char **argv){
  int numnodes=-1;
  int linknum = 0;
  char **topomatrix;
  Link ***linkmatrix;
  int maxpathsPerPair = MAXPATHSPERPAIR;
  
  char *type_list;
  int *failflags;
  FILE* fin;

  if(argc < 4){
    printf("Usage %s <topo> <tm> <wts> [fail_string]\n", NAME);
    exit(0);
  }

  char *topoFile = argv[1], *tmFile = argv[2], *wtFile = argv[3], *failString="";

  if(argc > 4){
    failString = argv[4];
    printf("\t Fail String: %s\n", argv[4]);
  }
  printf("Applying OSPF:\n\tTopo: %s\n\ttmFile: %s\n\twtFile: %s\n\tfailString %s\n", 
	 topoFile,  tmFile, wtFile, failString);


  fin = fopen(topoFile, "r");
  if( fin == NULL ) {
    printf("Cannot open %s\n", topoFile); exit(1);
  }
  numnodes = readtopo(fin, &topomatrix, &linkmatrix, &type_list, &linknum);

  
  double **optweights = get2ddouble(numnodes);
  read2dArray_file(numnodes, wtFile, optweights);
  loadweights(optweights, linkmatrix, numnodes);

  double **demands = get2ddouble(numnodes);
  read2dArray_file(numnodes, tmFile, demands);

  if(strlen(failString) > 0){
    failflags = (int *) malloc(linknum * sizeof(int));
    bzero(failflags, linknum * sizeof(int));
    readfailflags_fromline(&failflags, failString, numnodes, linknum);
    if (DEBUG) printfailflags(failflags, linknum, stdout);
    modify_topo(linkmatrix, topomatrix, failflags, numnodes, '&', linknum);
  }

  double **dist; char ***succ;
  floydwarshall(numnodes, topomatrix, linkmatrix, 1, &dist,  &succ);

  double ** util = getutilvalues(demands, succ, numnodes, linkmatrix);
  printutilvalues(numnodes, linknum, linkmatrix, util, topomatrix);

  double maxu = .0;
  int ind1,ind2;
  for(ind1=0; ind1<numnodes; ind1++)
    for(ind2=0; ind2<numnodes; ind2++)
      if(topomatrix[ind1][ind2] == '+' && 
	 util[ind1][ind2]>maxu) maxu = util[ind1][ind2];  
  printf("#\t Max Util: %g\n", maxu);

  free2ddouble(numnodes, util);
  free2ddouble(numnodes, demands);
  clearfloydmem(numnodes, succ, dist);
  if(strlen(failString) > 0)
    free(failflags);
}
