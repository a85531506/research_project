#define MAXLINELEN 1500

#ifndef DEBUG
#define DEBUG 0
#endif

#define MAXNUMNODES 100
#define MAXLINKNUM  10000
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
int kbest=20;
int Path_numberinS; 
extern char* optarg;
extern int optind;
char *bestcostfile;
char *append;

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

int FeasibilityCheck(Link*** linkmatrix, double ** path_preference_matrix, int ** path_specification_matrix, int n)
{
	int *path_length_matrix;
	int i;
	int j;
	
	path_length_matrix=(int *)malloc(sizeof(int *)*Path_numberinS);
	for(i=0; i<Path_numberinS; i++)
	{
		path_length_matrix[i]=0;
		for(j=0; j<n-1; j++)
		{
			
			if(path_specification_matrix[i][j+1]!=-1)
			{
				
				path_length_matrix[i]=path_length_matrix[i]+linkmatrix[path_specification_matrix[i][j]][path_specification_matrix[i][j+1]]->lat;
	
			}	
			else
			{
				break;
			}
		}
	}
	for(i=0; i<Path_numberinS; i++)
	{
		for(j=0; j<Path_numberinS; j++)
		{
			if(path_preference_matrix[i][j]==1)
			{

				
				if(path_length_matrix[i]>path_length_matrix[j])
				{
					return 0;
				}
				
			}

				else
				{
					if(path_preference_matrix[i][j]==-1)
					{
						if(path_length_matrix[i]<path_length_matrix[j])
				{
					return 0;
				}
					}
				}
		}
	}
free(path_length_matrix);
	return 1;
}

void InitPathSpecMatrix(int n, int ** initmatrix, char *filename)
{


char cmd[1000];
  sprintf(cmd, "tms2cread.pl %s", filename);

  FILE*fin = popen(cmd, "r");

  char line[MAXLINELEN];
  int indi1=0, linelen;
  int numcols=-1;
 int i;
  while(fgets(line, MAXLINELEN, fin) != NULL){
    linelen = strlen(line);
    if(linelen == MAXLINELEN) {
      printf("Too long a line: %d\n", linelen); abort();
    }else line[--linelen]=0;
    
    // printf("Parsing line [%s]\n", line);
    
    char temp[MAXLINELEN];
    char *temp2 = (char *)malloc(sizeof(char) * MAXLINELEN);
    char *c_ptr;

    strcpy(temp, line);
    char*tempc = temp;
    int ind2=0;
    while( (c_ptr = (char*)strtok_r(tempc, " ", &temp2)) != NULL){
      double d_ = atof(c_ptr);
      // printf("%d:%d val is %g\n", indi1, ind2, d_);
      if(ind2 > n || indi1>Path_numberinS) {
	printf("too many fields, mismatched topo-traf? this is ind2 %d, %d\n",ind2,n);
	exit(5);
      }
      initmatrix[indi1][ind2++] = d_;
      if(tempc != NULL) tempc= NULL;
    }
    if(numcols == -1) numcols = ind2;
    else if(numcols != ind2) { printf("cols mismatch\n"); exit(8);}
    indi1++;
  }
  if(indi1 != Path_numberinS) 
    { printf("rows mismatch found: %d has to be %d cols: %d\n", indi1, n, Path_numberinS); exit(9); }


  

}

void free2ddouble(int n, double** ptr){
  int indi1;
  for(indi1=0; indi1<n; indi1++)
    free(ptr[indi1]);
  free(ptr);
  return;
}

void **copy2ddouble(int n, double**dest, double**src){
  int indi1;
  //printf("came in\n");
  for(indi1=0; indi1<n; indi1++)
    memcpy(dest[indi1], src[indi1], n*sizeof(double));
 //printf("gotout\n");
}

void print2ddouble(int n, double **mat){
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++)
      printf("%g ", mat[indi1][ind2]);
    printf("\n");
  }
}

double ** get2ddouble(int n){
  int indi1;
  double ** dummy = (double **) calloc(n, sizeof(double*));
  for(indi1=0; indi1<n; indi1++){
    dummy[indi1] = (double*) calloc(n, sizeof(double));
    bzero(dummy[indi1], n * sizeof(double));
  }
  return dummy;
}

void add2ddouble(int n, double **dest, double **src){
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      dest[indi1][ind2] += src[indi1][ind2];
  return;
}
void divide2ddouble(int n, double **dest, double d){
  if( d <= 0) { printf("cannot divide by zero! "); exit(22); }
  int indi1, ind2;

  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      dest[indi1][ind2] /= d;
  return;
}

void get_bimodal_tm(int n, Link*** linkmatrix, int l, 
		    FILE* f, double**demandmatrix) {
  double weights[MAXNUMNODES][MAXNUMNODES];
  int indi1, ind2;

  if(f) fprintf(f, "Demands = [\n");
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++){
      if(indi1 == ind2) 
	weights[indi1][ind2] = 0;
      else 
	{
	  double x1 = drand48(), x2=drand48();
	  if(x1 > .5) {
	    if (DEBUG) printf("-");
	    weights[indi1][ind2] = x2*.1 + .8;
	  }
	  else{
	    if ( DEBUG) printf("+");
	    weights[indi1][ind2] = x2*.1 + .2;
	  }
	}
      if(f) fprintf(f, " %-9.4f", weights[indi1][ind2]*100);
      if(demandmatrix) (demandmatrix)[indi1][ind2] = weights[indi1][ind2]*100;
    }
    if ( DEBUG ) printf("\n");
    if(indi1 != n-1)
      if(f) fprintf(f, ";\n");
  }
  if(f) fprintf(f, "];\n");
}

void get_gravity_tm(int n, Link*** linkmatrix, int tot_links, FILE* f, 
		    double **demandmatrix){
  int *sum_cap = (int *) malloc( n * sizeof(int)), indi1, ind2;
  bzero(sum_cap, n*sizeof(int));

  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++) {
      Link *l = linkmatrix[indi1][ind2];
      sum_cap[indi1] += (l == NULL)? 0 : l->cap;
    }

  double *weights = (double *) malloc(n * sizeof(double));
  bzero(weights, n*sizeof(double));

  int allsum=0;
  for(indi1=0; indi1<n; indi1++) {
    weights[indi1] = (drand48()*0.9 + .1) *.2;
    allsum += sum_cap[indi1];
  }
  if ( DEBUG ) {
    printf("weights is [");
    for(indi1=0; indi1<n; indi1++)
      printf("%g ", weights[indi1]);
    printf("]\n");
    printf("allsum is %d\n", allsum);
  }

  if(f) fprintf(f, "Demands = [\n");
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++) {
      double demd=0;
      if(ind2 != indi1)
	demd = (weights[indi1] * sum_cap[indi1] * sum_cap[ind2])/ 
	  (allsum - sum_cap[indi1]);
      if(demandmatrix) (demandmatrix)[indi1][ind2] = demd;
      if(f) fprintf(f, " %-9.4f ", demd);
    }
    if(indi1 != n-1)
      if(f) fprintf(f, ";\n");
  }
  if(f) fprintf(f, "];\n");
  free(sum_cap);
  free(weights);
}

/* num_nodes, linkmatrix, num_links */
void print_matlab_topo(int n, Link *** linkmatrix, int tot_links) {
  FILE* matlab_topo = fopen("matlab_topo", "w");

  fprintf (matlab_topo, "n=%d;\nc=%d;\nl=%d;\n", n, n, tot_links);
  fprintf (matlab_topo, "Topo =[");

  int indi1=0, ind2=0;
  for(; indi1< n; indi1++) {
    fprintf(matlab_topo, "\t");
    for(ind2=0; ind2< n; ind2++) {
      Link* l = linkmatrix[indi1][ind2];
      int l_id = (l==NULL)? 0: l->id;
      fprintf(matlab_topo, "%3d ", l_id);
    }
    if (indi1 != n-1) fprintf(matlab_topo, ";\n");
  }

  fprintf(matlab_topo, "];\n");

  fprintf(matlab_topo, "Ends = [\n");
  for(indi1=0; indi1<n; indi1++){
    fprintf(matlab_topo, " %-3d", indi1+1);
    if(indi1 != n-1) 
      fprintf(matlab_topo, ";\n");
  }
  fprintf(matlab_topo, "];\n");

  int* alllink_cap = (int *) malloc(sizeof(int) * tot_links);
  bzero(alllink_cap, tot_links * sizeof(int));
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++){
      Link* l = linkmatrix[indi1][ind2];
      if( l == NULL) continue;
      alllink_cap[l->id] = l->cap;
    }

  fprintf(matlab_topo, "Capy = [\n");
  for(indi1=0; indi1<tot_links; indi1++){
    fprintf(matlab_topo, " %-7d", alllink_cap[indi1+1]);
    if(indi1 != tot_links-1)
      fprintf(matlab_topo, ";\n");
  }
  free(alllink_cap);
  fprintf (matlab_topo, "];\n");

  fclose(matlab_topo);
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


#define RANGESIZE 0.2
void getTMinmargin(int n, double **demands, double margin, double ***out_ptr){
  double **result = *out_ptr;
  int indi1,ind2;
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++){
      // proper defn. of margin
      double r1 = (drand48() * (margin-1))+1;

      // strict windows on either sides -- not good
      // double r1 = (drand48() * RANGESIZE)+margin-(RANGESIZE/2);
      int r2 = (drand48() > .5)?1:0;
      double val = demands[indi1][ind2];
      if(DEBUG)printf("val=%g, r1=%g, r2=%d, \n", val, r1, r2);

      if(r2) val /= r1; else val *= r1;
      if(DEBUG) printf("nval=%g\n", val);
      result[indi1][ind2] = val;
    }
}



#define WRITETM 4321
#define WRITEWEIGHT 4322
void __writeweights(int n, double **weights, FILE* fout, int what){
  int indi1, ind2;
  if (what == WRITETM) { fprintf(fout, "Demands = [\n"); }
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++)
	{
	if(weights[indi1][ind2]<0)
	{
		weights[indi1][ind2]=0;
	}
      fprintf(fout, "%g ", weights[indi1][ind2]);
	}
    if(what == WRITETM) fprintf(fout, ";");
    fprintf(fout, "\n");
  }
  if (what == WRITETM) { fprintf(fout, "];"); }
}

void writeweights(int n, double **weights, int w, int h, char *prefix, int isFail){
  char cmd[1000];

  if(h==w)    sprintf(cmd, "%s_%d", prefix, w);
  else    sprintf(cmd, "%s_%d_%d", prefix, w, h);



  if ( isFail ) strcat(cmd, ".fail_weights");
  else strcat(cmd, ".weight");

  if(append!=0)
  {
  strcat(cmd, append);
  }
  else
  {
  strcat(cmd, ".javab");
   }
  FILE* fout = fopen(cmd, "w");

  __writeweights(n, weights, fout, WRITEWEIGHT);

  fclose(fout);
}


void printfailflags(int *failflags, int l, FILE* fout){
  int a;
  fprintf(fout, "[");
  for(a=0; a<l; a++)
    if(failflags[a])
      fprintf(fout, "%4d ", a+1);
  fprintf(fout, "] ");
}

void freeallflags(int numlines, int **allfailflags){
  int indi1;
  for(indi1=0; indi1<numlines; indi1++)
    if(allfailflags[indi1]!=NULL) free(allfailflags[indi1]);
  free(allfailflags);
}

void readfailflags_fromline(int **flags_to_be_set, char *line, int fail, int l){
  char temp2[MAXLINELEN];
  char *c_ptr;
  strncpy(temp2, line, MAXLINELEN-1);
  char*tempc = temp2;
  char *temp = (char *)malloc(sizeof(char) * MAXLINELEN);

  // printf("Reading %s\n", temp2);
  int ind2=0;
  while( (c_ptr = (char*)strtok_r(tempc, " ", &temp)) != NULL){
    int d_ = atoi(c_ptr);
    // printf("found %d in line \n", d_);
    if(ind2 > fail) {
      printf("too many links, mismatched fail file\n");
      exit(5);
    }
    if(d_<1 || d_ > l) { printf("bad link %d to fail\n", d_); exit(31); }
    (*flags_to_be_set)[d_-1] = 1;
    ind2++;
    
    if(tempc != NULL) tempc= NULL;
  }
}

int readfailflags(int l, char* dir, int maxlines, int fail, int ***flags_ptr){

  (*flags_ptr) = (int **) malloc( maxlines * sizeof(int*) );
  bzero( *flags_ptr, maxlines * sizeof(int*));
  
  char cmd[1000];
  sprintf(cmd, "tms2cread.pl %s/fail_%d", dir, fail);
  printf("Running %s\n", cmd);
  FILE* fin = popen(cmd, "r");

  char line[MAXLINELEN];
  int indi1=0, linelen, numcols=-1;

  while(fgets(line, MAXLINELEN, fin) != NULL){

    if(indi1<maxlines){
      (*flags_ptr)[indi1] = (int *) malloc(l * sizeof(int));
      bzero( (*flags_ptr)[indi1], l*sizeof(int));
    }else
      break;

    linelen = strlen(line);
    if(linelen == MAXLINELEN) {
      printf("Too long a line: %d\n", linelen); abort();
    }else line[--linelen]=0;

    readfailflags_fromline( &( (*flags_ptr)[indi1] ), line, fail, l);

    if(DEBUG){
      printf("Failflag %d is ", indi1);
      printfailflags( (*flags_ptr)[indi1], l, stdout);
      printf("\n");
    }
    indi1++;
  }

  if(indi1!=maxlines) { printf("\n Couldn't read enough lines. Read %d/%d.\n", indi1, maxlines);}
  fclose(fin);

  return indi1;
}

/* given a bit vector over the links, 
 * modify the topo of links whose bit is set
 * -- serves both bringing down and up!
 */
void modify_topo(Link*** linkmatrix, 
		 char **topomatrix, int*failflags, int n,
		 char replacement, int size_failflags){
  int a, b;
  for(a=0; a<n; a++)
    for(b=0; b<n; b++)
      if(topomatrix[a][b] == '+' ||
	 topomatrix[a][b] == '&' ) /* some sort of link */
	{
	  int l_id = linkmatrix[a][b]->id;
	  if (l_id < 1 || l_id >size_failflags)
	    { printf("bad lid %d\n", l_id); exit(15); }
	  if (failflags[l_id - 1])
	    { if(DEBUG) printf("Taking out %d--%d\n", a, b);
	      topomatrix[a][b] = replacement; }
	}
}

/*
 * reads a standard topo file 
 * num-nodes, 
 * link capacity latency/weight
 * node type list
 * retval == numnodes
 */
int readtopo(FILE* fin, char*** topomatrix_ptr, 
	     Link**** linkmatrix_ptr, char **typelist_ptr, 
	     int* linknum_ptr){
  int linknum_ = 0;
  int numnodes_temp=-1;
  char line[MAXLINELEN];
  int linelen=0;
  int indi1, ind2, a, b;

  char *type_list_, **topomatrix_;
  Link*** linkmatrix_;
  /* read the topo in */
  while(fgets(line, MAXLINELEN, fin) != NULL ) {
    linelen = strlen(line); 
    if(linelen == MAXLINELEN) {
      printf("%s: Too long a line: %d\n", NAME, linelen); abort();
    }else line[--linelen]=0;

    if(line[0] == '#') continue;
    
    if(numnodes_temp == -1 && strstr (line, "NUM-NODES:") == line ){
      numnodes_temp = atoi ( line + strlen("NUM-NODES:") );
      if ( numnodes_temp > MAXNUMNODES) {
	printf("Too many nodes %d. Increase MAXNUMNODES from %d.\n", numnodes_temp, MAXNUMNODES);
	exit(1);
      }
      if(DEBUG) { printf("%s: numnodes got: %d\n", NAME, numnodes_temp); }

      topomatrix_ = (char **) calloc( numnodes_temp, sizeof(char*) );
      linkmatrix_ = (Link ***) calloc( numnodes_temp, sizeof(Link **));

      for(indi1 =0; indi1 < numnodes_temp; indi1++) {
	topomatrix_[indi1] = (char*) calloc(numnodes_temp,  sizeof(char));

	linkmatrix_[indi1] = (Link **) calloc(numnodes_temp, sizeof(Link*));
	bzero(linkmatrix_[indi1], numnodes_temp*sizeof(Link*));

	for(ind2=0; ind2 <numnodes_temp; ind2++)
	  topomatrix_[indi1][ind2] = '-';
	topomatrix_[indi1][indi1] = 'X';
      }


      type_list_ = (char *) calloc(numnodes_temp, sizeof(char));
      for(indi1=0; indi1<numnodes_temp; indi1++)
	type_list_[indi1] = ' ';

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
	printf("%s: topo entry[%d][%d] set, cap: [%s], lat: %f\n", 
	     NAME, a, b, cap, lat);

      if(! (is_in(a, 0, numnodes_temp-1) && is_in(b, 0, numnodes_temp-1))) {
	printf("%s: bad a: %d, b: %d\n", NAME, a, b); abort();
      }
      
      topomatrix_[a][b] = '+';
      topomatrix_[b][a] = '+';
      Link *l1 = (Link *) malloc(sizeof(Link));
      Link *l2 = (Link *) malloc(sizeof(Link));

      l1->id = ++linknum_;
      l2->id = ++linknum_;
      if( !strcmp(cap, "CC") ){
	l1->cap =  CC_CAP;
	l1->cap = l2->cap = CC_CAP;
      }else if( !strcmp(cap, "CP") ){
	l1->cap = CP_CAP;
	l1->cap = l2->cap = CP_CAP;
      }else if( !strcmp(cap, "PP") ){
	l1->cap  = PP_CAP;
	l1->cap = l2->cap = PP_CAP;
      }else {
	l1->cap = atof(cap);
	l1->cap = l2->cap = atof(cap);
      }
l1->lat = (double)lat;
      l1->lat = l2->lat = (double)lat;
      linkmatrix_[a][b] = l1;
      linkmatrix_[b][a] = l2;

    }else {
      if(DEBUG)      printf("%s: Ignoring [%s]\n", NAME, line);
    }
  }
  *(topomatrix_ptr) = topomatrix_;
  *(linkmatrix_ptr) = linkmatrix_;
  *(typelist_ptr) = type_list_;
  *(linknum_ptr) = linknum_;

  return numnodes_temp;
}



#define READTM  3000
#define READWT  3001
#define READFL  3002 /* weights optimized for failure */

void __readTM_WTS(int n, double **demandmatrix, FILE* fin)
{
  char line[MAXLINELEN];
  int indi1=0, linelen;
  int numcols=-1;


  while(fgets(line, MAXLINELEN, fin) != NULL){
    linelen = strlen(line);
    if(linelen == MAXLINELEN) {
      printf("Too long a line: %d\n", linelen); abort();
    }else line[--linelen]=0;
    
    //printf("Parsing line [%s]\n", line);
    
    char temp[MAXLINELEN];
    char *temp2 = (char *)malloc(sizeof(char) * MAXLINELEN);
    char *c_ptr;

    strcpy(temp, line);
    char*tempc = temp;
    int ind2=0;
    while( (c_ptr = (char*)strtok_r(tempc, " ", &temp2)) != NULL){
      double d_ = atof(c_ptr);
      // printf("%d:%d val is %g\n", indi1, ind2, d_);
      if(ind2 > n || indi1>n) {
	printf("too many fields, mismatched topo-traf?\n");
	exit(5);
      }
      demandmatrix[indi1][ind2++] = d_;
      if(tempc != NULL) tempc= NULL;
    }
    if(numcols == -1) numcols = ind2;
    else if(numcols != ind2) { printf("cols mismatch\n"); exit(8);}
    indi1++;
  }
  if( indi1 != numcols || indi1 != n) 
    { printf("rows mismatch found: %d has to be %d cols: %d\n", indi1, n, numcols); exit(9); }
  
}


// used for both demands and weights
void read2dArray_file(int n, char *filename, double **arr){
  char cmd[1000];
  sprintf(cmd, "tms2cread.pl %s", filename);

  FILE*fin = popen(cmd, "r"); 
 fclose(fin);
fin=popen(cmd, "r");
  if( fin == NULL ) { printf("Cannot exec cmd:[%s]\n", cmd); exit(7); }
  
  __readTM_WTS(n, arr, fin);

  fclose(fin);  
}


void readTMs(int n, char *tms_dir, int which, double **demandmatrix, char *tag){
  char cmd [1000];
  sprintf(cmd, "tms2cread.pl %s/%s_%d.m", tms_dir, tag, which);

  FILE* fin = popen(cmd, "r");
  if( fin == NULL ) { printf("Cannot exec cmd:[%s]\n", cmd); exit(7); }

  __readTM_WTS(n, demandmatrix, fin);

  fclose(fin);  
}


void printutilvalues(int n, int l, Link***linkmatrix, 
		     double **utils, char **topomatrix){
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      if(topomatrix[indi1][ind2] == '+'){
	int l_ = linkmatrix[indi1][ind2]->id;
	printf("%7.4g %3d [%d-->%d] cap %d\n", utils[indi1][ind2], l_, indi1, ind2, linkmatrix[indi1][ind2]->cap);
      }
}


void set_flags(char *c1, int* flags, int n){
  char *temp = (char *) malloc(PREDSTRING_LEN * sizeof(char));
  char *temp2 = temp;
  char *bootemp = (char *) malloc( PREDSTRING_LEN * sizeof(char));
  char *c_ptr, *bootemp2 = bootemp;
  strncpy(bootemp, c1, PREDSTRING_LEN-1);
  while( (c_ptr = (char *)strtok_r(bootemp2, ",", &temp)) != NULL){
    int x_ = atoi(c_ptr);
    if(x_ > n -1 ) { printf ("boo string %s in %s\n", c_ptr, c1); exit(3); }
    flags[x_] = 1;
    if(bootemp2 != NULL) bootemp2=NULL;
  }
  free (temp2);
  free (bootemp);
}


void concat_pred_lists(char *c1, char *c2, char **dest){
  if(DEBUG)  printf("concat [%s] [%s]\n", c1, c2);
  int *flags=(int*)malloc(MAXNUMNODES*sizeof(int));;
  bzero(flags, MAXNUMNODES * sizeof(int));
  
  set_flags(c1, flags, MAXNUMNODES);
  set_flags(c2, flags, MAXNUMNODES);

  char temps[100];
  bzero(temps, 100*sizeof(char));
  int indi1;
  for(indi1=0; indi1<MAXNUMNODES; indi1++)
    if(flags[indi1] == 1){
      if(strlen(temps) > 0)
	sprintf(temps+strlen(temps), ",");
      sprintf(temps+strlen(temps), "%d", indi1);
    }

  if(strlen(temps) > PREDSTRING_LEN)
    {printf ("after concat [%s] is too big\n", temps); exit(4);}
  strcpy(*dest, temps);
  free (flags);
}

void clearfloydmem(int n, char***succlist, double**distmatrix){
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++)
      free(succlist[indi1][ind2]);
    free(succlist[indi1]);
    free(distmatrix[indi1]);
  }
  free(distmatrix);
  free(succlist);
}


void printweights_matrix(int n, Link ***linkmatrix) {
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++)
      if(linkmatrix[indi1][ind2] != NULL)
	printf("%5.4g ", (linkmatrix[indi1][ind2]->lat));
      else
	printf("%5s ", "NA");
    printf("\n");
  }
}


double costfn(double util) {
  double retval;
  if (util <= (1.0/3))
    retval= util;
  else if (util <= (2.0/3))
    retval= (1.0/3) + 3 * (util - (1.0/3));
  else if (util <= (9.0/10))
    retval= (4.0/3) + 10 * (util - (2.0/3));
  else if (util <= 1.0)
    retval= (11.0/3) + 70 * (util - (9.0/10));
  else if (util <= (11.0/10))
    retval= (32.0/3) + 500 * (util - 1.0);
  else
    retval= (182.0/3) + 5000 * (util - 1.1);
  return retval;
}

#define FAILURE_BOUNDARY 0.6
double fcostfn(double util){
  double u_ = util/FAILURE_BOUNDARY;
  return costfn(u_);
}


#define MAXWT      20 /* from fortz et. al. */
#define MAXITER    5000 /* from fortz et. al. */

#define RANDWTS    100
#define INVCAPWTS  101
#define UNITWTS    102
#define GIVENWTS   103

void setweights(int n, Link*** linkmatrix, int type, int maxwt, double** given_weights){
  int indi1, ind2;

  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      if(linkmatrix[indi1][ind2] != NULL)
	switch(type){
	case RANDWTS:
	  linkmatrix[indi1][ind2]->lat = (int)(drand48() * (maxwt-1))+1;
	  break;
	case INVCAPWTS:
	  linkmatrix[indi1][ind2]->lat = (int)(CC_CAP/(linkmatrix[indi1][ind2]->cap));
	  break;
	case UNITWTS:
	  linkmatrix[indi1][ind2]->lat = 1;
	  break;
	case GIVENWTS:
	  linkmatrix[indi1][ind2]->lat = given_weights[indi1][ind2];
	  break;
	default:
	  printf("unknow wt type %d\n", type);
	  exit(14);
	}
  /* check */
  if(DEBUG) printweights_matrix(n, linkmatrix);
}




/* somewhat vague, but the weights are hidden inside linkmatrix[a][b]->lat */
int floydwarshall(int n, char **topomatrix, Link*** linkmatrix,
		  int switch_get_equal_wt_paths, double ***dist_ptr,
		  char ****succ_ptr){
  int indi1, ind2, numiter;

  /* shortest distance matrix */
  double** distances, **distances_t_;
  char*** successors, ***successors_t_;

  distances = (double **) calloc(n, sizeof(double));
  distances_t_ = (double **) calloc(n, sizeof(double));

  /* successors strings "a,b,c" => multiple equivalent upstream nodes */
  successors = (char ***) calloc(n, sizeof(char**));
  successors_t_ = (char ***) calloc(n, sizeof(char**));

  for(indi1=0; indi1<n; indi1++) {
    distances[indi1] = (double *) calloc(n, sizeof(double));
    bzero(distances[indi1], n * sizeof(double)); /* 0 => no path! */

    distances_t_[indi1] = (double *) calloc(n, sizeof(double));
    bzero(distances_t_[indi1], n * sizeof(double)); /* 0 => no path! */

    successors[indi1] = (char **) calloc(n, sizeof(char*));
    bzero(successors[indi1], n * sizeof(char*)); /* not set yet */

    successors_t_[indi1] = (char **) calloc(n, sizeof(char*));
    bzero(successors_t_[indi1], n * sizeof(char*)); /* not set yet */

    for(ind2=0; ind2<n; ind2++){
      int x_ = PREDSTRING_LEN * sizeof(char);
      successors_t_[indi1][ind2] = 
	(char *) malloc(x_);
      successors[indi1][ind2] = 
	(char *) malloc(x_);

      bzero(successors_t_[indi1][ind2], x_);
      bzero(successors[indi1][ind2], x_);
    }
  }

  /* init distances and successors */
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      if(topomatrix[indi1][ind2] == '+'){
	distances[indi1][ind2] = linkmatrix[indi1][ind2]->lat;
	if(DEBUG)
	printf ("init dist %d:%d is %g\n", indi1, ind2, distances[indi1][ind2]);
	char temp[10];
	sprintf(temp, "%d", ind2);
	strncpy(successors[indi1][ind2], temp, PREDSTRING_LEN-1);
      }else {
	distances[indi1][ind2] = -1;
	sprintf(successors[indi1][ind2], "none");
      }

  if(DEBUG) {
    printf("After init\n\n\n");
    print_dist_pred(n, distances, successors);
  }

  for(numiter=0; numiter<n; numiter++){
    for(indi1=0; indi1<n; indi1++)
      for(ind2=0; ind2<n; ind2++){
	double x_ = distances[indi1][numiter] + distances[numiter][ind2];

	if(indi1!=ind2 &&
	   distances[indi1][numiter] >0 &&
	   distances[numiter][ind2] >0 && 
	   (distances[indi1][ind2] == -1 || 
	     distances[indi1][ind2] >= x_ 
	    )){

	  /* new values */
	  if(distances[indi1][ind2] == x_  && switch_get_equal_wt_paths){
	    concat_pred_lists(successors[indi1][ind2], successors[indi1][numiter], 
			      &successors_t_[indi1][ind2]);
	    if(DEBUG)
	      printf("DUP %d->%d, new succ %s\n", indi1, ind2, successors_t_[indi1][ind2]);
	  }
	  else
	    strcpy(successors_t_[indi1][ind2], successors[indi1][numiter]);

	  distances_t_[indi1][ind2] = x_;
	    
	}else {
	  /* copy old values */
	  distances_t_[indi1][ind2] = distances[indi1][ind2];
	  strcpy(successors_t_[indi1][ind2], successors[indi1][ind2]);
	}
      }


    /* swap the matrices */
    double ** dummy = distances;
    distances = distances_t_;
    distances_t_ = dummy;

    char*** c_dummy = successors;
    successors = successors_t_;
    successors_t_ = c_dummy;
  }
  if(DEBUG) {
    printf("At end\n\n\n");
    print_dist_pred(n, distances, successors);
  }
  /* Sanity check - see if graph is connected */
  int retval=0;
  for(indi1=0; indi1<n; indi1++){
    for(ind2=0; ind2<n; ind2++)
      if(indi1 != ind2 && distances[indi1][ind2] < 0) {
	printf("#### graph is disconnected\n");
	retval=1; break;
      }
    if(retval) break;
  }

  *dist_ptr = distances;
  *succ_ptr = successors;

  /* free the temps */
  clearfloydmem(n, successors_t_, distances_t_);
  return retval; /* 0 => success, 1 => disconnection */
}

void unloadweights(double** wts, Link*** links, int n){
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      if(links[indi1][ind2] != NULL)
	wts[indi1][ind2] = links[indi1][ind2]->lat;
}

void loadweights(double** wts, Link*** links, int n){
  int indi1, ind2;
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      if(links[indi1][ind2] != NULL)
	links[indi1][ind2]->lat = wts[indi1][ind2];
}



#define BIGHASHSIZE     16 /* bits */
#define SMALLHASHSIZE   10 /* bits */
void resethash (unsigned int *hashptr, int hashsize ){
  if(hashptr == NULL) { printf ("incoming hash ptr is null\n"); exit(16); }
  bzero(hashptr, (1<<(hashsize-3)));
}
int __checknset (unsigned int *hashptr, int hashsize, int index){
  if (index < 0 || index > (1<<hashsize)-1){
    printf("bad index %d, given hashsize %d\n", index, hashsize); exit(17); 
  }

  unsigned int high = index >> 5;
  unsigned int low = (index - (high << 5));

  if(DEBUG) printf("index %d --> high=%u, low=%u ", index, high, low);

  unsigned int hval = ((hashptr[high]) & ( 1<<low)) >> low;
  hashptr[high] |= (1<<low);

  if(DEBUG) printf("hval=%u\n", hval);
  return hval;
}
void freehash(unsigned int *hashptr){
  free(hashptr);
}
unsigned int* createhash( int hashsize_bits ){
  unsigned int * ptr= (unsigned int *) malloc( (1<<(hashsize_bits-3)) ); /* 2^hashsize_bits bits */
  resethash(ptr, hashsize_bits);
  return ptr;
}
unsigned int fnv32_buf(void *buf, size_t len, unsigned int hval){
    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

    /*
     * FNV-1 hash each octet in the buffer
     */
    while (bp < be) {
	hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	hval ^= (unsigned int)*bp++;
    }

    /* return our new hash value */
    return hval;
}

int dohash(double **wtlist, int n, int l){
  int vecsize=0, indi1, ind2;
  int mywtvector[MAXLINKNUM];
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++)
      if(wtlist[indi1][ind2] > 0)
	mywtvector[vecsize++] = wtlist[indi1][ind2];
  if(0 && vecsize != l){
    printf("mismatch num links has to be %d, is %d\n", l, vecsize); exit(18);
  }
  return fnv32_buf(mywtvector, l*4, 0);
}

int checknset(double **wtlist, int n, int l, unsigned int* hashptr, int hashsize){

  int x_ = dohash(wtlist, n, l) & ((1<<hashsize) - 1);

  //printf("index is %d\n", x_);
  
  return __checknset(hashptr, hashsize, x_);
}

void setflags_fortree(int n, int target, double*** demands, 
		      char ***successors, char *tree, int howmany){
  int indi1, ind2, tmind;
  bzero(tree, n * sizeof(char));
  for(tmind=0; tmind<howmany; tmind++)
    for(indi1=0; indi1<n; indi1++)
      if( demands[tmind][indi1][target] > 0) 
	tree[indi1] = 1;

  /* check if everybody is already on the tree */
  int everyone_on_tree=1;
  for(indi1=0; indi1<n; indi1++)
    if(indi1 != target && tree[indi1] !=1) { everyone_on_tree=0; break;}
  if(everyone_on_tree) return;


  // drudge thru and add those ferrying demand for others
  int more, *flags = (int*) malloc(n * sizeof(int));
  do{
    more = 0;
    for(indi1=0; indi1<n; indi1++){
      if(tree[indi1] != 1) continue;
      bzero(flags, n*sizeof(int));
      set_flags(successors[indi1][target], flags, n);
      for(ind2=0; ind2<n; ind2++)
	if(flags[ind2] && (tree[ind2] <= 0)) {
	  tree[ind2] = 1;
	  more = 1;  /* check once more because this guy is 
		      * just added onto the tree */
	}
    }
  }while(more);
  free(flags);

  if(DEBUG) printf("Target %d\t Nodes on Tree=", target);
  for(indi1=0; indi1<n; indi1++)
    if(tree[indi1]>0) if(DEBUG) printf("%d, ", indi1);
  if(DEBUG) printf("\n");

  return;
}


double **getutilvalues(double **demandmatrix, char ***succmatrix,
		       int n, Link ***linkmatrix){
  int indi1, ind2, ind3;

  double **utilmatrix = get2ddouble(n);
  double *subload = (double *) malloc(n * sizeof(double));
  int** Tflags = (int **) malloc(n * sizeof(int**));
  int* numflags = (int *) malloc(n*sizeof(int));

  for(indi1=0; indi1<n; indi1++)
    Tflags[indi1] = (int*) malloc(n* sizeof(int));
  
  for(indi1=0; indi1<n; indi1++){

    /* convert all load going towards indi1 into utilization
     * hop-distance on the tree rooted at indi1
     */
    for(ind2=0; ind2<n; ind2++){
      subload[ind2] = demandmatrix[ind2][indi1];
      bzero(Tflags[ind2], n* sizeof(int));
      set_flags(succmatrix[ind2][indi1], Tflags[ind2], n);
      numflags[ind2]=0;
      for(ind3=0; ind3<n; ind3++)
	if(Tflags[ind2][ind3]) numflags[ind2] += 1;
    }
    subload[indi1]=0;

    if(DEBUG) 
    for(ind2=0; ind2<n; ind2++){
      printf("Tflags[%d] \t", ind2);
      for(ind3=0; ind3<n; ind3++) 
	printf(" %3d", Tflags[ind2][ind3]);
      printf("\tTot: %d\n", numflags[ind2]);
    }

    /* go ahead and relax */
    double maxremload;
    int pass_=0;
    do{
      if(DEBUG) {
	printf("For %d Pass %d: ", indi1, pass_++);
	for(ind2=0; ind2<n; ind2++)
	  printf(" %g", subload[ind2]);
	printf("\n");
      }
      for(ind2=0; ind2<n; ind2++){
	if(subload[ind2] == 0 || ind2==indi1) continue;
	for(ind3=0; ind3<n; ind3++)
	  if(Tflags[ind2][ind3]) {
	    double l_ = (subload[ind2]/numflags[ind2]);
	    subload[ind3] += l_;
	    utilmatrix[ind2][ind3] += l_;
	  }
	subload[ind2] = 0;
      }
      if(DEBUG) 
	printf("\tPass %d: load sinked at %d is %g\n", 
	       pass_-1, indi1, subload[indi1]);
      subload[indi1] = 0;
      maxremload = -1;
      for(ind2=0; ind2<n; ind2++)
	if(maxremload < subload[ind2]) maxremload = subload[ind2];
    }while(maxremload > 0);
  }

  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++){
      if(linkmatrix[indi1][ind2] == NULL) continue;
      utilmatrix[indi1][ind2] /= (linkmatrix[indi1][ind2]->cap);
    }

  free(subload);
  free(numflags);
  for(indi1=0; indi1<n; indi1++)
    free(Tflags[indi1]);
  free(Tflags);
  return utilmatrix;
}

double **getutilvalues_over_manyTMS(double ***tms, char ***successors, 
				    int  n, Link ***linkmatrix, int howmany){
  int tmind;
  double **avgutilmatrix = get2ddouble(n);
  for(tmind=0; tmind<howmany; tmind++){
    double **u_ = getutilvalues(tms[tmind], successors, n, linkmatrix);
    add2ddouble(n, avgutilmatrix, u_);
    free2ddouble(n, u_);
  }
  divide2ddouble(n, avgutilmatrix, howmany);
  return avgutilmatrix;
}


void printcritical(int l, char*curr_critical_flags, double* util){
  int indi1;
  printf("[");
  for(indi1=0; indi1<l; indi1++)
    if(curr_critical_flags[indi1]==1)
      printf("\t%3d:%6.4g \n", indi1, util[indi1]);
  printf("]\n");
}


double ospfcost(double **demandmatrix, char ***succmatrix, int n,
		Link ***linkmatrix, double (*fnptr)(double) ){
  int indi1, ind2;
  double **utilmatrix = getutilvalues(demandmatrix, succmatrix, n, linkmatrix);
  double totcost = 0.0;

  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++){
      if(linkmatrix[indi1][ind2] == NULL) continue;
      double load_ = utilmatrix[indi1][ind2];
      if(DEBUG)
	printf("%d-->%d load_ %g, cost: %g\n", 
	       indi1, ind2, load_, fnptr(load_));
      //      totcost += fnptr(load_);  -- srikanth tot-cost fix 09/06/05
      totcost += fnptr(load_) * linkmatrix[indi1][ind2]->cap;
    }
  free2ddouble(n, utilmatrix);

  return totcost;
}

double maxutil_demands(double **demandmatrix, char***succmatrix, 
		       int n, Link*** linkmatrix){
  int indi1, ind2;
  double **utilmatrix = getutilvalues(demandmatrix, succmatrix, n, linkmatrix);
  
  double maxutil=-1.0;
  for(indi1=0; indi1<n; indi1++)
    for(ind2=0; ind2<n; ind2++){
      if(linkmatrix[indi1][ind2] == NULL) continue;
      double load_ = utilmatrix[indi1][ind2];
      if(load_ > maxutil)
	maxutil = load_;
    }
  free2ddouble(n, utilmatrix);
 
  return maxutil;
}

/* Given a current critical set, update it according to the latest weights
 * for the given TM
 * such that the set is no larger than 'size'
 */
void update_critical_set(int n, Link ***linkmatrix, int l,
			 char **topomatrix,
			 double ***tms, int size, 
			 char *curr_critical_flags){
  double *fail_link_maxutil = (double *) malloc(sizeof(double) * l);
  int indi1;

  
  /* compute max-util in topo without this link */
  int *failflags = (int *) malloc( l * sizeof(int));
  double **dist;
  char ***succ;
  for (indi1=0; indi1<l; indi1++){
    bzero(failflags, l * sizeof(int));
    failflags[indi1] = 1;
    modify_topo(linkmatrix, topomatrix, failflags, 
	       n, '&', l);
    
    floydwarshall(n, topomatrix, linkmatrix,
		  1,
		  &dist, &succ);
    fail_link_maxutil[indi1] = maxutil_demands(tms[0], succ, 
					      n, linkmatrix);
    modify_topo(linkmatrix, topomatrix, failflags, 
	       n, '+', l);
    clearfloydmem(n, succ, dist);
  }

  printf("Begin: ");
  printcritical(l, curr_critical_flags, fail_link_maxutil);
  printf("\n");
  free (failflags);

  /* keep adding in new links to the set until no more change   */
  int change;
  do{
    printf("inside\n");
    change = 0;

    double u_inside_min_ = 100;
    int inside_min_id_ = -1;
    double u_bar_ = 0.0;
    int currsize = 0;
    for(indi1=0; indi1<l; indi1++)
      if(curr_critical_flags[indi1] == 1) {
	double u_ = fail_link_maxutil[indi1];
	u_bar_ += u_;
	currsize++;
	if(u_<u_inside_min_) {
	  u_inside_min_ = u_;
	  inside_min_id_ = indi1;
	}
      }

    if(currsize > 0) u_bar_ /= currsize;
    printf("u_bar = %g, currsize= %d\n", u_bar_, currsize);

    double u_outside_max_ = 0.0;
    int outside_max_id_ = -1;
    for(indi1=0; indi1<l; indi1++)
      if(curr_critical_flags[indi1] != 1 &&
	 fail_link_maxutil[indi1] > u_outside_max_){
	u_outside_max_ = fail_link_maxutil[indi1];
	outside_max_id_ = indi1;
      }

    if(currsize < size || 
       u_outside_max_ > u_bar_) {
      /* add it in */
      currsize++;
      printf("Adding in %d\n", outside_max_id_ );
      change = 1;
      curr_critical_flags[outside_max_id_] = 1;

      /* kick out somebody */
      if(currsize > size){
	currsize--;
	if(inside_min_id_< 0) { printf("no min_id\n"); exit(32); }
	else curr_critical_flags[inside_min_id_] = 0;
      }
    }
    
  }while(change == 1);

  printf("End: ");
  printcritical(l, curr_critical_flags, fail_link_maxutil);
  printf("\n");

  free(fail_link_maxutil);
  
}





void computeOSPFCosts( double ***tms, int howmany, int n, 
		       char ***successors, Link***linkmatrix, 
		       double (*fnptr)(double),
		       double *currcost, double *currutil, 
		       char *curr_critical_flags, int l, char **topomatrix,
		       int optimize_over_fails) {
  *currutil = -1;
  *currcost = 0;
  int tmind;  

  /* cost is sum over multiple tms */
  for(tmind=0; tmind<howmany; tmind++){
    (*currcost) += ospfcost(tms[tmind], successors, n, linkmatrix, fnptr);
    double u_temp_ = maxutil_demands(tms[tmind], successors, n, linkmatrix);
    if(u_temp_  > *currutil) *currutil = u_temp_;
  }
  (*currcost) /= howmany;

  if(optimize_over_fails){
    if(DEBUG)printf("%d tms cost is %g\n", howmany, *currcost);
    /* compute failuer cost over the critical set */
    double failure_cost = 0;
    int *failflags = (int*) malloc(l * sizeof(int));
    double **dist;
    char ***succ;
    int indi1, currsize=0;
    for(indi1=0; indi1<l; indi1++)
      if(curr_critical_flags[indi1] != 1) continue;
      else{
	currsize++;
	bzero(failflags, l * sizeof(int));
	failflags[indi1] = 1;
	modify_topo(linkmatrix, topomatrix, failflags,
		    n, '&', l);
	floydwarshall(n, topomatrix, linkmatrix,
		      1,
		      &dist, &succ);
	failure_cost += ospfcost(tms[0], succ, n, linkmatrix, fnptr);
	/* scaled down */
	//failure_cost += ospfcost(tms[0], succ, n, linkmatrix, fcostfn); 

	clearfloydmem(n, succ, dist);
	modify_topo(linkmatrix, topomatrix, failflags,
		    n, '+', l);
      }
    free(failflags);

    if(DEBUG)printf("failure cost on the first tm is %g over %d links\n", 
	   failure_cost, currsize);
    
    if(currsize > 0) {
      failure_cost /= currsize;
      (*currcost) = .5 * ( (*currcost) + failure_cost );
    }
    if(DEBUG)printf("total cost is %g\n", *currcost);
  }
}
		       

int setFirstBestk(double ncost, double nutil, double *bestkcost, double *bestkutil, int *tempnumberinlist, int n, double **tempweights, double ***bestKweights, int * checked)
	{
			int index=0;
			int index2;
			while((ncost<bestkcost[index] || (ncost == bestkcost[index] && nutil < bestkutil[index]))&& index<(*tempnumberinlist))
			{
				index++;
			}
			if(index==(*tempnumberinlist))
			{
			copy2ddouble(n,bestKweights[(*tempnumberinlist)],tempweights);
			checked[(*tempnumberinlist)]=0;
			bestkcost[(*tempnumberinlist)]=ncost;
			bestkutil[(*tempnumberinlist)]=nutil;
			(*tempnumberinlist)++;
			return 1;

			}
			else
			{
				for(index2=(*tempnumberinlist); index2>index; index2--)
				{
					copy2ddouble(n,bestKweights[index2],bestKweights[index2-1]);
					checked[index2]=checked[index2-1];
					bestkcost[index2]=bestkcost[index2-1];
					bestkutil[index2]=bestkutil[index2-1];
				}
				copy2ddouble(n,bestKweights[index],tempweights);
				checked[index]=0;
				bestkcost[index]=ncost;
				bestkutil[index]=nutil;	
			(*tempnumberinlist)++;
				return 1;
			}
		       


}

/* tms is an array of 'howmany' traffic matrices, over which optimal 
 * weights are to be computed
 */
int checkBestk(double ncost, double nutil, double *bestkcost, double *bestkutil, int *tempnumberinlist, double***bestKweights, double **tempweights, int n, int * checked)
{
			int index2;
			int index=0;
			if(ncost<bestkcost[index] || (ncost==bestkcost[index] && nutil<bestkutil[index]))
			{
				
				while((ncost<bestkcost[index] || (ncost == bestkcost[index] && nutil < bestkutil[index]))&& index<(*tempnumberinlist))
				{
					index++;
					
				}
			
			
			if(index==(*tempnumberinlist))
			{
			copy2ddouble(n,bestKweights[(*tempnumberinlist)-1],tempweights);
			checked[(*tempnumberinlist)-1]=0;
			bestkcost[(*tempnumberinlist)-1]=ncost;
			bestkutil[(*tempnumberinlist)-1]=nutil;
			//printbestcost(bestkcost);	
			return 1;
			}
			else
			{
				
				for(index2=(*tempnumberinlist)-1; index2>index; index2--)
				{
					
					copy2ddouble(n,bestKweights[index2],bestKweights[index2-1]);
					checked[index2]=checked[index2-1];
					bestkcost[index2]=bestkcost[index2-1];
					bestkutil[index2]=bestkutil[index2-1];
					
				}
				copy2ddouble(n,bestKweights[index],tempweights);
				checked[index]=0;
				bestkcost[index]=ncost;
				bestkutil[index]=nutil;
				
				return 1;
			}
		       }
	
}
void printbestcost(double *bestkcost)
{
int indi1;
FILE * filedescribe;
char *filename="./checkbestcosts";

filedescribe=fopen(filename,"a");
printf("\n*********************\n");
for(indi1=0; indi1<kbest; indi1++)
{

	printf("%g\n",bestkcost[indi1]);
}

fclose(filedescribe);
}

void setoptimalweights(double ***tms, int n, int l, double ** path_preference_matrix, int ** path_specification_matrix,
		       Link***linkmatrix, char **topomatrix, double** given_weights,
		       int which, int howmany, 
		       int optimize_over_fails, char *outFilePrefix,
			double cost_threshold)
{

/* TO DO LIST:
A- AN ARRAY OF THE K BEST SOLUTIONS FOUND SO FAR. 
B- AN ARRAY OF THE K BEST COSTS CORRESPONDING TO THOSE SOLUTIONS.
C- AN ARRAY INDICATING IF THE NEIGHBORHOOD OF THAT SOLUTION HAS BEEN EXPLORED OR NOT. 
D- TO UPDATE LIST YOU NEED TO CHECK THE NEIGHBORHOOD OF EACH OF THE SOLUTIONS VISITED. 
*/
  struct timeval now;
  gettimeofday(&now, NULL);
  printf("+ BeginOPT %d %d\n", now.tv_sec, now.tv_usec);

  int smallhashsize = 20 * l; /* 20 times the number of arcs */
  int smallhashsizebits = 0;
  while( (1<<smallhashsizebits) < smallhashsize )
    smallhashsizebits++;

  printf("HASH: Big %d bits, Small %d bits\n", BIGHASHSIZE, smallhashsizebits);

  unsigned int* bighash   = createhash(BIGHASHSIZE);
  unsigned int* smallhash = createhash(smallhashsizebits);

  int indi1, ind2, tmind;
  int numiterations = MAXITER;

  /* initialize RNG */
  printf("Seeding at %d\n", getpid());
  srand48(getpid());

  /* Set link weights */
  if (given_weights)
  {
    setweights(n, linkmatrix, GIVENWTS, MAXWT, given_weights);
  }
  else
  {
	kbest=20;
    setweights(n, linkmatrix, RANDWTS, MAXWT, 0);
  }

  double **currweights = get2ddouble(n);
  double **tempweights = get2ddouble(n);
  double **bestweights = get2ddouble(n);

//this is a k dimensional matrix of the best weights seen so far. 
double ***bestKweights=(double ***)malloc(kbest * sizeof(double **));
for(indi1=0; indi1<kbest; indi1++)
{
	bestKweights[indi1]=get2ddouble(n);
}



  char *curr_critical_flags = (char *)malloc(l * sizeof(char));
  bzero(curr_critical_flags, l * sizeof(char));

  double percentsample=.2;
  double currcost, currutil;
  double *bestkcost=(double *)malloc(sizeof(double)*kbest);
  double *bestkutil=(double *)malloc(sizeof(double)*kbest);
  int *checked=(int *)malloc(sizeof(int)*kbest);
int numberinlist=1;


for(indi1=0; indi1<kbest; indi1++)
{

bestkcost[indi1]=1<<20;
checked[indi1]=1;
bestkutil[indi1]=1<<30;
}
checked[0]=0;

  /* set currweights */
  for(indi1=0; indi1<n; indi1++)
    for(ind2=indi1+1; ind2<n; ind2++)
      if(linkmatrix[indi1][ind2] != NULL){
	currweights[indi1][ind2] = linkmatrix[indi1][ind2]->lat;
	currweights[ind2][indi1] = linkmatrix[ind2][indi1]->lat;
	bestKweights[0][indi1][ind2]=currweights[indi1][ind2];
	bestKweights[0][ind2][indi1]=currweights[ind2][indi1];
      }else{
	currweights[indi1][ind2] = -1;
	currweights[ind2][indi1] = -1;
	bestKweights[0][indi1][ind2]= -1;
	bestKweights[0][ind2][indi1]= -1;
      }

/*What has been done so far? 1- an array bestkweights was added, this arry is what contains the k best weights that were seen so far. The first element of the array has been set to the current wieght which we are going to be exploring. 2- all the other elements have been marked as checked, since they do not contain any set of viable weights whose neighborhood we would want to check, (the vector checked marks the weights that have/have not been checked so far. 3- we also have a vector kbestcost which contains the cost of the best paths that we have seen so far. number in list indicates the number of weights that have been added to the list so far.*/

  /* compute beginning cost */
  double **distances;
  char ***successors;

  /* set state for these weights in the hash table */
printf("this is it\n");
  checknset(currweights, n, l, bighash, BIGHASHSIZE);
printf("this is not it\n");

  /* compute all-pairs shortest paths */
  floydwarshall(n, topomatrix, linkmatrix, 1, &distances, &successors);
  computeOSPFCosts(tms, howmany, n, successors, linkmatrix, costfn,
		   &currcost, &currutil, 
		   curr_critical_flags, l, topomatrix, 
		   optimize_over_fails);
  clearfloydmem(n, successors, distances);
  distances=0;
  successors=0;
bestkcost[0]=currcost;
bestkutil[0]=currutil;
  printf("Begin with cost %g, max-util %g\n\n", currcost, currutil);
  int lastiterimproved = -1;
int tempnumberinlist=numberinlist;

  /* Note: currweights is the local best 
   * tempweights is the new point we are evaluating
   * bestweights is the global best until now
   */

  // initialize bestweights
  copy2ddouble(n, bestweights, currweights);

  double bestcost = currcost;
 double bestutil = currutil;
	
  int indkbest;
/*what I want to do from this point on is to look at the neighborhood of the k best weights that have been looked at so far.*/
  // ready to roll!

  for(indi1=0; indi1<numiterations; indi1++) {
 numberinlist=tempnumberinlist;
//Behnaz Arzani: We need a modification here to add a loop that would go through the k best links that we have visited so far and look at their neighborhoods. (FYI: so far the program operation has not been changed, I have added some variables but that is it.) 

for(indkbest=numberinlist-1; indkbest>=0; indkbest--)
{
      printf("man alan in tu hastam:DP dumbass\n");
	if(checked[indkbest]==0)
	{
	
	//HAVE TO MODIFY CODE HERE TO ACCOUNT FOR THE K BEST WEIGHTS THAT HAVE BEEN OBSERVED SO FAR. 
    /* new neighborhood exploration -> reset secondary hash table */
    resethash(smallhash, smallhashsizebits);

    int improved=0; /* did local optima improve? */

/*Behnaz Arzani: So far I have added a loop that goes through the k best weights that have been observed so far and if their one hop neighborhood has not yet been searched, then it will be explored. */

    copy2ddouble(n,currweights,bestKweights[indkbest]);

    checked[indkbest]=1;
    /* init temp weights and the linkmatrix */
    copy2ddouble(n, tempweights, currweights);
    loadweights(currweights, linkmatrix, n);

    /* update critical set of link failures */
    if(optimize_over_fails && indi1 > 0 && 
       indi1 % UPDATE_CRITICAL_SET_FREQ == 0)
      update_critical_set(n, linkmatrix, l,
			  topomatrix, tms, CRITICAL_SET_SIZE,
			  curr_critical_flags);

    /* single weight changes */
/*In this part the entire one hop neighborhood of of tempweights is checked.*/
    int a, b,c;
    int newindex=0;
    for(a=0; a<n; a++)
      for(b=0; b<n; b++){
	if(topomatrix[a][b] != '+') continue;
	for(c=1; c<= MAXWT; c++){
	  if( c == tempweights[a][b] || 
	      drand48() > percentsample) continue;
	newindex++;
	
	  double old = tempweights[a][b];
	  if(DEBUG)
	    printf("changing %d--%d from %g to %d", a, b, old, c);
	  tempweights[a][b] = c;

	  /* have to change the cost elsewhere too, hack */
	  linkmatrix[a][b]->lat = c;
	
	  /* if this wt. vector is not already present in the hash table */
	  if(checknset(tempweights, n, l, bighash, BIGHASHSIZE) == 0 &&
	     checknset(tempweights, n, l, smallhash, smallhashsizebits) == 0) {

	/*before checking the cost we have to do a feasibility check*/
	if(given_weights && Path_numberinS>0)
	{
	 if(FeasibilityCheck(linkmatrix, path_preference_matrix, path_specification_matrix,n)!=1)
		{
		 tempweights[a][b] = old;
	 	 linkmatrix[a][b]->lat = old;
	continue;
		}
	}
	    /* evaluate the cost of the new wt. vector */
	   
	    double ncost, nutil;
	    floydwarshall(n, topomatrix, linkmatrix, 1, &distances, &successors);
	    computeOSPFCosts(tms, howmany, n, successors, linkmatrix, costfn,
			     &ncost, &nutil, curr_critical_flags, l, topomatrix,
			     optimize_over_fails);
	    clearfloydmem(n, successors, distances);
	    distances = 0; successors = 0;

	    if(DEBUG)
	      printf("\t new cost is %g, util is %g", ncost, nutil);
	    
/*Behnaz Arzani, this should be modified to look inside the list instead of looking just at the single best cost.*/
	   if(tempnumberinlist<kbest)
		{
			
			improved=setFirstBestk(ncost, nutil, bestkcost, bestkutil, &tempnumberinlist, n, tempweights, bestKweights, checked);
	          
		/*let us assume that the list is so far sorted from the worst answer seen so far to the best.*/
			
		}
		else
		{
			
		improved= checkBestk(ncost, nutil, bestkcost, bestkutil, &tempnumberinlist, bestKweights, tempweights, n, checked);
			
		}
	
//printf("this is where i am at now\n");
	if(bestkcost[tempnumberinlist-1]<cost_threshold)
	{
      copy2ddouble(n, bestweights, bestKweights[tempnumberinlist-1]);
      loadweights(bestweights, linkmatrix, n);
      if(DEBUG) printweights_matrix(n, linkmatrix);
      bestcost = bestkcost[tempnumberinlist-1];
      bestutil = bestkutil[tempnumberinlist-1];
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	goto threshold_reached;
	}


	    if(ncost < currcost ||
	       (ncost == currcost && nutil < currutil)
	       ){
	      if(DEBUG)
		printf("\t Good");
	      copy2ddouble(n, currweights, tempweights);
	      currcost = ncost;
	      currutil = nutil;
	      improved = 1;
	    }
	  }

	  if(DEBUG)
	    printf("\n");
	    
	  /* return tempweights to pristine state */
	  tempweights[a][b] = old;
	  linkmatrix[a][b]->lat = old;
	}

      }
    /* at end, tempweights is the original point, 
     * linkmatrix is same as original, 
     * currweights is the local best from single weight change
     */
    



//un loop balaie inja edame dare, un iteration checker e
 




    /* evenly balancing flows */
    double *totdem = (double*) malloc(n * sizeof(double));
    bzero(totdem, n * sizeof(double));
    for(tmind=0; tmind<howmany; tmind++)
      for(a=0; a<n; a++)
	for(b=0; b<n; b++)
	  totdem[a] += tms[tmind][b][a];

    char *flag_on_tree = (char*) malloc(n * sizeof(char));

    /* get current shortest paths */
    floydwarshall(n, topomatrix, linkmatrix, 1, &distances, &successors);
    if(DEBUG) print_dist_pred(n, distances, successors);

    double **utilmatrix = getutilvalues_over_manyTMS
      (tms, successors, n, linkmatrix, howmany);

    double **storedweights = get2ddouble(n); /* temporary store while probing */
    for(a=0; a<n; a++){
      if(totdem[a] == 0) continue;
       /* for each demand node */

      /* find all nodes ferrying demand TO this node */
      setflags_fortree(n, a, tms, successors, flag_on_tree, howmany);
      for(b=0; b<n; b++){
	if(flag_on_tree[b] <= 0 || b == a) continue;
	if(drand48() > percentsample) continue;
	/* for each node sending traffic to 'a' */
	if(DEBUG)  printf("At tree node %d", b);
	
	double util_thres = (drand48()*.75)+.25;
	if(DEBUG)  printf("\tutilthresh is %g\n", util_thres);

	int *neigh_in_setB = (int *) malloc(n * sizeof(int));
	int *neigh_outside = (int *) malloc(n * sizeof(int));
	bzero(neigh_outside, n * sizeof(int));
	bzero(neigh_in_setB, n * sizeof(int));
	
	/* apply the lightly-loaded constraint first */
	for(c=0; c<n; c++)
	  if(topomatrix[b][c] == '+')
	    if(utilmatrix[b][c] <= util_thres) neigh_in_setB[c]=1;
	    else neigh_outside[c] = 1;


	/* all nodes in set B should have a shorter weighted path to target
	 * than those outside
	 */
	double outside_min_distance = (MAXWT * l)+1;
	for(c=0; c<n; c++){
	  if(neigh_outside[c] != 1) continue;
	  double t_ = linkmatrix[b][c]->lat + distances[c][a];
	  if(t_  < outside_min_distance) outside_min_distance = t_;
	}

	for(c=0; c<n; c++){
	  if(neigh_in_setB[c] != 1) continue;
	  double t_ = linkmatrix[b][c]->lat + distances[c][a];
	  if( t_ > outside_min_distance ) {
	    neigh_outside[c] = 1; neigh_in_setB[c] = 0;
	  }
	}

	double max_inset_distance = -1;
	double min_inset_distance = MAXWT*l + 1;
	for(c=0; c<n; c++){
	  if(neigh_in_setB[c] != 1) continue;
	  double d_ca = distances[c][a];
	  if(c == a) d_ca = 0;
	  if(d_ca < min_inset_distance) min_inset_distance = d_ca;
	}
	if(DEBUG) printf("\t min_inset_distance = %g\n", min_inset_distance);

	/* if set B is not empty */
	if(min_inset_distance < MAXWT*l + 1) {
	  /* throw out neighbors too far away from min */
	  for(c=0; c<n; c++){
	    if(neigh_in_setB[c] != 1) continue;
	    double d_ca = distances[c][a];
	    if(c == a) d_ca = 0;
	    if(d_ca > min_inset_distance + MAXWT-1)
	      { neigh_in_setB[c] = 0; continue; }
	    else if(d_ca > max_inset_distance)
	      max_inset_distance = d_ca;
	  }
	  if(DEBUG) printf("\t max_inset_distance = %g\n", max_inset_distance);

	  /* if set B has more than one element */
	  if( max_inset_distance > -1 && 
	      max_inset_distance != min_inset_distance ) {
	    /* remember tempweights is pristine */
	    copy2ddouble(n, storedweights, tempweights); 

	    double tot_desired = max_inset_distance + 1;
	    for(c=0; c<n; c++) {
	      if(neigh_in_setB[c] != 1) continue;
	      double d_ca = distances[c][a];
	      if(c == a) d_ca = 0;
	      double w_ = tot_desired - d_ca;
	      if(w_ < 1.0 || w_ > MAXWT)
		{ printf("boo nwt sucks %g\n", w_); exit(18); }
	      if(DEBUG) 
		printf("\t BAL %d -> %d from %g to %g (%g)\n",
		       b, c, tempweights[b][c], w_, distances[c][a]);
	      tempweights[b][c]  = w_;
	    }

	    double **dist1;
	    char ***succ1;
	    /* if this is a new wt vector */
	    if(checknset(tempweights, n, l, bighash, BIGHASHSIZE) == 0 && 
	       checknset(tempweights, n, l, smallhash, smallhashsizebits) == 0) {
	      loadweights(tempweights, linkmatrix, n);
	      double ncost, nutil;
	      floydwarshall(n, topomatrix, linkmatrix, l, &dist1, &succ1);
	      computeOSPFCosts(tms, howmany, n, succ1, linkmatrix, costfn,
			       &ncost, &nutil,
			       curr_critical_flags, l, topomatrix,
			       optimize_over_fails);
	      clearfloydmem(n, succ1, dist1);
	      if(DEBUG)	      
	      printf("\t new cost:%g, util: %g", ncost, nutil);

		 if(tempnumberinlist<kbest)
		{

			improved=setFirstBestk(ncost, nutil, bestkcost, bestkutil, &tempnumberinlist, n, tempweights, bestKweights, checked);
	          
		/*let us assume that the list is so far sorted from the worst answer seen so far to the best.*/
			
		}
		else
		{
			
		improved= checkBestk(ncost, nutil, bestkcost, bestkutil, &tempnumberinlist, bestKweights, tempweights, n, checked);
		
		}
	      if(ncost < currcost ||
		 (ncost == currcost &&  nutil < currutil) ){
		if(DEBUG) 
		  printf("\t\t\t Good\n");
		copy2ddouble(n, currweights, tempweights);
		currcost = ncost;
		currutil = nutil;
		improved = 1;
 
	      }
	    }
	    if(DEBUG) 
	      printf("\n");

	    /* restore things to their pristine state */
	    copy2ddouble(n, tempweights, storedweights);
	    loadweights(tempweights, linkmatrix, n);
	  }
	}
	
	free(neigh_in_setB);
	free(neigh_outside);
      }
	
    }

    free (totdem);
    free (flag_on_tree);
    free2ddouble(n, utilmatrix);
    free2ddouble(n, storedweights);
    clearfloydmem(n, successors, distances);
 
















    /* diversify */
    if(
       /* an obvious check, but not in paper, so why bother */
       // (percentsample == 1 && !improved) ||
	(indi1-lastiterimproved > 3000 && !improved) ){
      if(DEBUG) printf("Random perturbation needed!\n");
      printf("Perturb- Cost: %g, Util: %g \t", currcost, currutil);
      for(a=0; a<n; a++)
	for(b=0; b<n; b++){

	  /* perturb 10% of the links */
	  if(topomatrix[a][b]!= '+' || drand48()>.1) continue;

	  /* weight change is uniform in [-2, +2] */
	  int perturb_ = (int)(drand48() * 5)-2;
	  currweights[a][b] += perturb_;
	  
	  /* clamp them */
	  if(currweights[a][b] < 1) currweights[a][b] = 1;
	  if(currweights[a][b] > MAXWT) currweights[a][b] = MAXWT;
	}
      loadweights(currweights, linkmatrix, n);
      floydwarshall(n, topomatrix, linkmatrix, 1, &distances, &successors);
      computeOSPFCosts(tms, howmany, n, successors, linkmatrix, costfn,
		       &currcost, &currutil, curr_critical_flags, l, topomatrix,
		       optimize_over_fails);
      clearfloydmem(n, successors, distances);
      printf("----> Cost: %g, Util: %g\n", currcost, currutil);

      percentsample = .1; /* reset, becomes 20% later on */
    }

    if(improved) { 
      lastiterimproved = indi1;
      /* reset secondary hash table */;
      resethash(smallhash, smallhashsizebits);
    }

    /* update percentsample */
    if(improved)
      percentsample /= 3;
    else
      percentsample *= 2;
    if(percentsample > 1.0) percentsample = 1.0;
    if(percentsample < 0.01) percentsample = 0.01;

    
    printf("Iter %d: new cost=%g, percentsample=%g costhresh=%g\n", 
	   indi1, currcost, percentsample, cost_threshold);
     if(tempnumberinlist<kbest)
		{

			improved=setFirstBestk(currcost, currutil, bestkcost, bestkutil, &tempnumberinlist, n, currweights, bestKweights, checked);
	          
		/*let us assume that the list is so far sorted from the worst answer seen so far to the best.*/
			
		}
		else
		{
			
		improved= checkBestk(currcost, currutil, bestkcost, bestkutil, &tempnumberinlist, bestKweights, currweights, n, checked);
		
		}
    /* check if improvement in global cost has happened */
    if(currcost < bestcost ||
       (currcost == bestcost && currutil < bestutil) ) {
     printf("Iter %d, GREAT (%g, %g) --> (%g, %g)\n", 
	     indi1, bestcost, bestutil, currcost, currutil);
      copy2ddouble(n, bestweights, currweights);
      loadweights(bestweights, linkmatrix, n);
      if(DEBUG) printweights_matrix(n, linkmatrix);
      bestcost = currcost;
      bestutil = currutil;
    }

    /* Break out early, once cost goes below target threshold */
    if (bestcost <= cost_threshold)
    {
	
       printf("Best cost (%g) is now below threshold (%g)\n", bestcost, cost_threshold);
	  copy2ddouble(n, bestweights, bestKweights[tempnumberinlist-1]);
      loadweights(bestweights, linkmatrix, n);
      if(DEBUG) printweights_matrix(n, linkmatrix);
      bestcost = bestkcost[tempnumberinlist-1];
      bestutil = bestkutil[tempnumberinlist-1];
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	goto threshold_reached;
       break;
    }
  }
  }
}
threshold_reached:
  /* compute "best" values */
   copy2ddouble(n, bestweights, bestKweights[numberinlist-1]);
  loadweights(bestweights, linkmatrix, n);
  printweights_matrix(n, linkmatrix);

  printf("TMS:%d-%d Ended with cost %g, max-util=%g\n", 
	 which, which+howmany-1, bestcost, bestutil);


/*for(indi1=0; indi1<n;indi1++)
{
	for(ind2=0; ind2<n; ind2++)
	{
		
		printf("%g\t",bestweights[indi1][ind2]);
	}
	printf("\n");
}*/

 if(bestcostfile==0)
	{
		bestcostfile="./bestcost";
	}
  FILE *bestfile;
  char *bestfilename=bestcostfile;
  bestfile=fopen(bestfilename, "w");
  fprintf(bestfile,"%g",bestcost);
  fclose(bestfile);
	
  writeweights(n, bestweights, which, which+howmany-1, outFilePrefix, optimize_over_fails);

  free2ddouble(n, currweights);
  free2ddouble(n, tempweights);
  free2ddouble(n, bestweights);

  freehash(bighash);
  freehash(smallhash);

  free(bestkutil);
  free(checked);
  free(bestkcost);

     for(indi1=0; indi1<kbest; indi1++)
	{
		  free2ddouble(n, bestKweights[indi1]);
	}
  gettimeofday(&now, NULL);
  printf("+ EndOPT %d %d\n", now.tv_sec, now.tv_usec);

}
