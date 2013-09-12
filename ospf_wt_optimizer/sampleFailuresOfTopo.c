#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define NAME "sampleFailuresOfTopo"
#include "topo_help.h"

/* 
 * Usage: ./sampleFailuresOfTopo topo numLinksToFail howManySamples [outdir]
 *
 * topo is a generic topo file
 * numLinksToFail is the # of links that simultaneously fail
 * howManySamples is the # of failure samples
 * output gets a file called "fail_%d", where %d is numLinksToFail
 */

/*
 *  Given a topology and a certain number of links that have to 
 *    simultaneously fail;
 *  Samples the topo uniformly at random and yields failure instances
 *  For each failure instance, run floydwarshall to make sure that 
 *    the graph is not disconnected
 *
 */


#define OUTDIRSIZE 300
int main(int argc, char **argv){
  FILE* fin, *fout;
  int numnodes,linknum;
  char **topomatrix, *type_list;
  Link*** linkmatrix;
  int switch_gen_gravity=1, switch_gen_bimodal=0;
  
  if(argc < 4 || argc > 5){
    printf("Usage: %s <topo> <numLinksToFail> <howManySamples> [OUTDIR]\n", NAME);
    exit(0);
  }
  
  if (DEBUG)
    printf("outside optind is %d, argc is %d, opt is %s\n", optind, argc, argv[optind]);
  
  char *topoFile = argv[1];
  int how_many_links_fail = atoi(argv[2]), 
    how_many_repetitions = atoi(argv[3]);

  char outfile [OUTDIRSIZE];
  if ( argc > 4 ){
    if( snprintf(outfile, OUTDIRSIZE, "%s/fail_%d", argv[4], how_many_links_fail) > OUTDIRSIZE )
      { printf("outfile name overflow\n"); exit(434); }
    
    if ( (fout = fopen(outfile, "w")) == NULL )
      { printf("can't open %s for write\n", outfile); exit(435); }
  }else{
    strcpy(outfile, "stdout");
    fout = stdout;
  }
  
  printf("sampleFailuresOfTopo: \n\tTopo %s\n\tFail %d links at once\n\tNum Repeats %d\n\tOutFile %s\n",
	 topoFile, how_many_links_fail, how_many_repetitions, outfile);


  // open topo
  fin = fopen(topoFile, "r");
  if( fin == NULL ) {
    printf("Cannot open %s\n", topoFile); exit(1);
  }
  numnodes = readtopo(fin, &topomatrix, &linkmatrix, &type_list, &linknum);


  if(how_many_links_fail > linknum/3){
    how_many_links_fail = linknum/3;
    fprintf(stderr, "This topo only has %d links, resetting to %d fails!\n",
	    linknum, how_many_links_fail);
  }


  {
    int ind1, nextseed;
    int nogood;
    int *failflags = (int *) malloc( (int)(linknum) * sizeof(int));
    double **dist;
    char ***succ;


    /* special case: dump out all possible cases of 1 link failure */
    if(how_many_links_fail == 1)
      for(ind1=0; ind1<linknum; ind1++){
	bzero(failflags, linknum*sizeof(int));
	failflags[ind1] = 1;
	modify_topo(linkmatrix, topomatrix, failflags, numnodes, '&', linknum);
	if(floydwarshall(numnodes, topomatrix, linkmatrix, 
			 0, 
			 &dist, &succ) == 1){
	  /* disconnected */
	  fprintf (fout, "#### ");
	  printfailflags(failflags, linknum, fout);
	  fprintf(fout, "\n");
	}
	else {
	  if(DEBUG) printf("Tearing down ");
	  printfailflags(failflags, linknum, fout);
	  fprintf(fout, "\n");
	}
	clearfloydmem(numnodes, succ, dist);
	modify_topo(linkmatrix, topomatrix, failflags, numnodes, '+', linknum);
      }

    else /* randomly sample space */
      for(ind1=0,nextseed=0; ind1<how_many_repetitions; ind1++, nextseed++){
	srand48(getpid()+nextseed); 
	nogood = 0;
	bzero(failflags, linknum * sizeof(int));
	
	int to_fail = how_many_links_fail;
	
	do{
	  int l_ = (int)(drand48() * linknum)+1;
	  if ( l_ < 1 || l_ > linknum) 
	    { printf("bad l_=%d tot links=%d\n", l_, linknum/2); exit(12); }
	  
	  if (failflags[l_-1] ==1) {
	    if(DEBUG) printf("\n\t Link l_=%d of %d already torn! Redo.. \n", 
			     l_, linknum);
	    continue;
	  }else {
	    failflags[l_-1] = 1;
	    to_fail--;
	  }
	}while(to_fail > 0);
	
	modify_topo(linkmatrix, topomatrix, failflags, numnodes, '&', linknum);
	if(floydwarshall(numnodes, topomatrix, linkmatrix, 
			 0, 
			 &dist, &succ) == 1){
	  /* disconnected */
	  fprintf (fout, "#### ");
	  printfailflags(failflags, linknum, fout);
	  fprintf(fout, "\n");
	  ind1 -= 1;
	  nogood = 1;
	}
	
	if(!nogood) {
	  if(DEBUG) printf("Tearing down ");
	  printfailflags(failflags, linknum, fout);
	  fprintf(fout, "\n");
	}
	clearfloydmem(numnodes, succ, dist);
	modify_topo(linkmatrix, topomatrix, failflags, numnodes, '+', linknum);
      }
    free(failflags);
  }
}
