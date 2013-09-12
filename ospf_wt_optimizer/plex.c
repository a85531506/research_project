
#define _POSIX_C_SOURCE 201301L
#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

enum log_level
{
   log_info,
   log_warning,
   log_error,
   log_none
};

float costthresh=0;
const enum log_level active_log_level = log_warning;

int log_message(enum log_level level, const char* format, ...)
{
   if (level < active_log_level)
      return 0;

   va_list args;
   va_start(args, format);
   int r = vfprintf(stderr, format, args);
   va_end(args);
   fflush(stderr);
   return r;
}




struct child_params
{
   int id;
   char* cmd;
};

typedef int (*child_main)(struct child_params* job);


void child_params_destroy(struct child_params* params)
{
   if (params->cmd)
      free(params->cmd);
}


struct job_set
{
   size_t n_children;
   pid_t* procs;
   struct child_params* params;
   child_main child_main_function;
};

void job_set_create(struct job_set* js, size_t n_children, child_main child_main_function)
{
   js->n_children = n_children;
   js->procs = (pid_t*)malloc(n_children * sizeof(pid_t));
   js->params= (struct child_params*)malloc(n_children * sizeof(struct child_params));
   js->child_main_function = child_main_function;
   size_t i;
   for ( i = 0; i < n_children; ++i)
   {
      js->procs[i] = 0;
      js->params[i].id = i;
      js->params[i].cmd = 0;
   }
}

void job_set_destroy(struct job_set* js)
{
   free(js->procs);
   size_t i;
   for ( i = 0; i < js->n_children; ++i)
   {
      child_params_destroy(&js->params[i]);
   }

   free(js->params);
   js->n_children = 0;
}

pid_t job_set_spawn_one(struct job_set* js, struct child_params* params)
{
   pid_t p = fork();
   if (p == 0)
   {
      exit( js->child_main_function(params) );
   }
   else if (p == -1)
   {
      log_message(log_error, "Error spawning child %d\n", params->id);
   }

   return p;
}


int job_set_spawn_all(struct job_set* js)
{
   size_t n_successful = 0;
   size_t i;
   for ( i = 0; i < js->n_children; ++i)
   {
      pid_t p = job_set_spawn_one(js, &js->params[i]);
      
      if (p != -1)
      {
         js->procs[i] = p;
         ++n_successful;
      }
   }

   return n_successful;
}


int job_set_await_first_finisher(struct job_set* js, int* status)
{
   pid_t child = wait(status);

   int first_finisher = -1;
   size_t i;
   for(i = 0; i < js->n_children; ++i)
   {
      if (js->procs[i] == child)
      {
         first_finisher = i;
		printf("!!!!!!!!!!!!!this guy is the first finisher %d!!!!!!!!\n",i);
         break;
      }
   }

   return first_finisher;
}

int job_set_terminate(struct job_set* js)
{
   // Murder your darlings
size_t i;
   for (i = 0; i < js->n_children; ++i)
   {
      if (js->procs[i] > 0)
      {
         log_message(log_info, "Killing %d (%d)\n", i, js->procs[i]);
         kill(js->procs[i], SIGTERM);
      }
   }

   // Reap the wait status for all remaining children
   pid_t child;
   int status;
   size_t n_killed = 0;
   while ((child = wait(&status)) > 0)
   {
      ++n_killed;
      log_message(log_info, "Child stopped (%d, %d, %d)\n", 
                  child, WIFEXITED(status), WEXITSTATUS(status));
   }

   return n_killed;
}




int my_child_main(struct child_params* job)
{
   log_message(log_info, "Child %d: start\n", job->id);

   system(job->cmd ? job->cmd : "true");
   sleep(2);

   log_message(log_info, "Child %d: done\n", job->id);

   return EXIT_SUCCESS;
}


int do_initial_run(struct job_set* js, struct child_params* initial_params)
{
   /*pid_t initial_run = job_set_spawn_one(js, initial_params);
   if (initial_run <= 0)
   {
      log_message(log_error, "Initial run failed to start\n");
      return 0;
   }

   int status;
   pid_t finished = waitpid(initial_run, &status, 0);
   if (finished != initial_run)
   {
      log_message(log_error, "Initial run failed (%d)\n", status);
      return 0;
   }

   log_message(log_info, "Initial run ok\n");*/
   FILE * bestcost;
   char *filename="./bestcost";
   bestcost=fopen(filename,"r");
   fscanf(bestcost,"%g",&costthresh);
  costthresh=1.1*costthresh;
   FILE *file;
   char * esm="./behnaz";
   file=fopen(esm, "w");
   fprintf(file, "%g",costthresh);
   fclose(file);
   fclose(bestcost);
   return 1;
}


void child_params_init_cmd(struct child_params* params, int fileindex)
{
   char* cmd = 0;
	size_t cmd_size=0;
  if(costthresh!=0)
  {
    cmd_size = asprintf(&cmd, "./getOptOSPFWeights -m %s %s %s %d %d %s%d %s %s %f %s%d %s%d",
                              "./sprint.topo",
                              "./sample_topos/topo1_tms",
                              "gengrav",
                              4,
                              544,
                              "./newweights",
				fileindex,
                              "./pathspecifier",
                              "./pathpref",
				costthresh,
				"./bestcost",
				fileindex,
				".result",
				fileindex
				);
}
else
{
	 cmd_size = asprintf(&cmd, "./getOptOSPFWeights -m %s %s %s %d %d %s %s %s %f %s %s",
                              "./sprint.topo",
                              "./sample_topos/topo1_tms",
                              "gengrav",
                              4,
                              0,
                              "./weights.1",
                              "./pathspecifier",
                              "./pathpref",
				costthresh,
				"./bestcost",
				".result");
}


   size_t max_size = sysconf(_SC_ARG_MAX);

   if (cmd_size > max_size)
   {
      log_message(log_warning, "Command string for %d too long (%d > %d)", params->id, cmd_size, max_size);
      params->cmd = 0;
      free(cmd);
      return;
   }

   params->cmd = cmd;
}



int main(void)
{
   const size_t n_kids = 2;
   struct job_set js;
   job_set_create(&js, n_kids, &my_child_main);

   struct child_params initial_params;
   initial_params.id = 0;
   child_params_init_cmd(&initial_params,-1);

   int initial_run_ok = do_initial_run(&js, &initial_params);
   if (!initial_run_ok)
      return EXIT_FAILURE;

size_t i;
   for (i = 0; i < n_kids; ++i)
   {
      child_params_init_cmd(&js.params[i],i);
   }
int number;
   number=job_set_spawn_all(&js);
   
   
   log_message(log_info, "Parent: waiting\n");

   int status;
   int first_finisher = job_set_await_first_finisher(&js, &status);
   
   log_message(log_info, "Job %d finished (%s)\n", first_finisher, (WIFEXITED(status) ? "ok" : "fail"));

   job_set_terminate(&js);
   job_set_destroy(&js);

   return EXIT_SUCCESS;
}
