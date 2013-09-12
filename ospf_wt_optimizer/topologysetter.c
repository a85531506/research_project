#define MAXLINELEN 1500

#ifndef DEBUG
#define DEBUG 0
#endif

#define MAXNUMNODES 100
#define MAXLINKNUM  200
#define MAXPATHSPERPAIR 100
#define PREDSTRING_LEN 100

#define CC_CAP 10000 /* Mbps */
#define CP_CAP 2500  /* Mbps */
#define PP_CAP 2500  /* Mbps */

#define NUMTMS 1
#define BASETM 1
#define NUMFAILURES 1
#define NUMREPEATS 2

#define OPTIMIZE_OVER_FAILURES 0
#define UPDATE_CRITICAL_SET_FREQ 10
#define CRITICAL_SET_SIZE 5

/*Defining the number of path in the set S*/
#define Path_numberinS 1289
extern char* optarg;
extern int optind;

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct link{
  int id;
  double lat;
  int cap; /* Mbps */
} Link;

/* node in a linked list of paths */
typedef struct path{
  int path_vector[MAXNUMNODES]; 
  int numhops;
  double lat;
  struct path* next; 
  struct path* snext;
} Path;

typedef struct node{
  int id;
  double x;
  double y;
} Node;

inline int is_in( int a, int min, int max) {
  return ((a>=min) && (a<= max));
}

void parse_type_line(char c, char* line, char* type_list, int n){
  char *ct, *ctbig = line;
  while((ct = strstr(ctbig, " ")) != NULL){
    while (isspace(*ct) )  ct++;
    if ( *ct == 0) { 
	printf("reached end of line \n");
        break; 
    }
    int entity = atoi (ct);
    
    if(entity>n || entity<0  || type_list[entity] != ' ') {
	printf("entity: %d, type_list: %c\n",
		entity, type_list[entity]);
      abort();
	}
    
    type_list[entity] = c;

    ctbig = ct;
  }
}



int readtopo(FILE* fin, int*** topomatrix_ptr, 
	     Link**** linkmatrix_ptr, char **typelist_ptr, 
	     int* linknum_ptr){
  int linknum_ = 0;
  int numnodes_temp=-1;
  char line[MAXLINELEN];
  int linelen=0;
  int ind1, ind2, a, b;
  FILE *filedescriptor;
  char topologyfilename[]="./sample_topos/matlabtopology";

  char *type_list_;
int **topomatrix_;
  Link*** linkmatrix_;
  /* read the topo in */
  while(fgets(line, MAXLINELEN, fin) != NULL ) {
    linelen = strlen(line); 
    if(linelen == MAXLINELEN) {
      printf(" Too long a line: %d\n", linelen); abort();
    }else line[--linelen]=0;

    if(line[0] == '#') continue;
    
    if(numnodes_temp == -1 && strstr (line, "NUM-NODES:") == line ){
      numnodes_temp = atoi ( line + strlen("NUM-NODES:") );
      if ( numnodes_temp > MAXNUMNODES) {
	printf("Too many nodes %d. Increase MAXNUMNODES from %d.\n", numnodes_temp, MAXNUMNODES);
	exit(1);
      }
      if(DEBUG) { printf("numnodes got: %d\n",numnodes_temp); }

      topomatrix_ = (int **) calloc( numnodes_temp, sizeof(int*) );
      linkmatrix_ = (Link ***) calloc( numnodes_temp, sizeof(Link **));

      for(ind1 =0; ind1 < numnodes_temp; ind1++) {
	topomatrix_[ind1] = (int*) calloc(numnodes_temp,  sizeof(int));

	linkmatrix_[ind1] = (Link **) calloc(numnodes_temp, sizeof(Link*));
	bzero(linkmatrix_[ind1], numnodes_temp*sizeof(Link*));

	for(ind2=0; ind2 <numnodes_temp; ind2++)
	  topomatrix_[ind1][ind2] = 0;
	topomatrix_[ind1][ind1] = 0;
      }


      type_list_ = (char *) calloc(numnodes_temp, sizeof(char));
      for(ind1=0; ind1<numnodes_temp; ind1++)
	type_list_[ind1] = ' ';

      if(DEBUG) print_topo_matrix(topomatrix_, numnodes_temp);
    }else if(numnodes_temp>-1 && strstr(line, "dummy") == line) {
      parse_type_line('d', line, type_list_, numnodes_temp);
    }else if(numnodes_temp>-1 && strstr(line, "ingress") == line) {
      parse_type_line('i', line, type_list_, numnodes_temp);
    }else if(numnodes_temp>-1 && strstr(line, "egress") == line) {
      parse_type_line('e', line, type_list_, numnodes_temp);
    }
    else if (numnodes_temp>-1 && 
	     strstr(line, "LINK:") == line){
      float lat;
      char cap[10];
      char* ctmp = line + strlen("LINK:");

      if ( sscanf(ctmp, " %d %d %s %f", &a, &b, cap, &lat) != 4){
	printf("bad line in topo file: [%s]\n", line);
	exit(343);
      }
      
      if(DEBUG)
	printf(" topo entry[%d][%d] set, cap: [%s], lat: %f\n", 
	      a, b, cap, lat);

      if(! (is_in(a, 0, numnodes_temp-1) && is_in(b, 0, numnodes_temp-1))) {
	printf("bad a: %d, b: %d\n", a, b); abort();
      }
      
      topomatrix_[a][b] = 1;
      topomatrix_[b][a] = 1;
      Link *l1 = (Link *) malloc(sizeof(Link));
      Link *l2 = (Link *) malloc(sizeof(Link));

      l1->id = ++linknum_;
      l2->id = ++linknum_;
      if( !strcmp(cap, "CC") ){
	l1->cap = l2->cap = CC_CAP;
      }else if( !strcmp(cap, "CP") ){
	l1->cap = l2->cap = CP_CAP;
      }else if( !strcmp(cap, "PP") ){
	l1->cap = l2->cap = PP_CAP;
      }else {
	l1->cap = l2->cap = atof(cap);
      }

      l1->lat = l2->lat = (double)lat;
      linkmatrix_[a][b] = l1;
      linkmatrix_[b][a] = l2;

    }else {
      if(DEBUG)      printf(" Ignoring [%s]\n", line);
    }
  }
 
filedescriptor= fopen(topologyfilename, "w");

	for(ind1=0; ind1<numnodes_temp; ind1++)
	{
		for(ind2=0; ind2<numnodes_temp; ind2++)
		{
			fprintf(filedescriptor,"%d\t",topomatrix_[ind1][ind2]);	
		}
	fprintf(filedescriptor,"\n");
	}
fclose(filedescriptor);
  return numnodes_temp;
}

int main()
{
char topoFile[]="./sprint.topo";
FILE * fin;
 fin = fopen(topoFile, "r");
int **topomatrix;
char *type_list;
int linknum;
Link*** linkmatrix;
readtopo(fin, &topomatrix, &linkmatrix, &type_list, &linknum);
}
