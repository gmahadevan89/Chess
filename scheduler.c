#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "scheduler.h"

struct Replay_list_elem *rlist_head=NULL, *rlist_tail=NULL;

struct Replay_list_elem *get_new_rle (void) {
  struct Replay_list_elem *temp = (struct Replay_list_elem *)malloc(sizeof(struct Replay_list_elem));
  temp->et_head=NULL;
  temp->et_tail=NULL;

  if (rlist_head == NULL)
    rlist_head = temp;
  temp->prev=rlist_tail;
  if (rlist_tail != NULL)
    rlist_tail->next=temp;
  rlist_tail=temp;
  temp->next=NULL;
  
  return temp;
}

void add_se_to_rlist (struct Replay_list_elem *rle, struct State_elem st_) {
  rle->st = st_;
}

void add_enabled_thread (struct Replay_list_elem* rle, int t_) {
  struct Enab_th_elem *temp = (struct Enab_th_elem*)malloc(sizeof(struct Enab_th_elem));
  temp->t = t_;
  if (rle->et_head == NULL) {
    rle->et_head = temp;
  }
  temp->prev=rle->et_tail;
  if (rle->et_tail != NULL)
    (rle->et_tail)->next=temp;
  rle->et_tail=temp;
  temp->next=NULL;
}

void print_et_list (struct Enab_th_elem *head) {
  struct Enab_th_elem *et = head;
  while (et != NULL) {
    printf ("%d ",et->t);
    et = et->next;
  }
}

void print_rlist (void) {
  printf ("[SCHEDULER] Printing rlist:\n");
  struct Replay_list_elem *rle = rlist_head;  
  while (rle != NULL) {
    printf ("%d %d %d\t",rle->st.cur_tid, rle->st.sync_pt, rle->st.choice);
    print_et_list (rle->et_head);
    printf ("\n");
    rle = rle->next;
  }
}    

int get_next_replay (void) {
  //printf ("Generating next replay...\n");
  int new_choice = -1;
  struct Replay_list_elem *rle = rlist_tail;
  while (rle != NULL) {
    struct Enab_th_elem *et = rle->et_head;
    while (et != NULL && et->t <= rle->st.choice)
      et = et->next;
    if (et != NULL) {
      new_choice = et->t;
      break;
    } else {
      rle = rle->prev;
    }
  }
  
  //printf ("new_choice = %d\n",new_choice);

  if (new_choice == -1) {
    return 1;
  }
  // "Create" a new replay list. In reality, just print the old one
  // except where the change is to be made.
  FILE *fp = fopen ("replay","w");
  struct Replay_list_elem *oldlist=rlist_head;
  int b=0;
  while (1) {
    if (oldlist == rle) {
      fprintf (fp,"%d\n",new_choice);
      break;
    } else {
      fprintf (fp,"%d\n",oldlist->st.choice);
    }
    oldlist = oldlist->next;
  }
  
  fclose (fp);
  return 0;
    
}

void free_replay_list (void) {
  struct Replay_list_elem *p = rlist_head, *f;
  while (p != NULL) {
    f = p;
    p = p->next;
    free (f);
  }
  rlist_head = rlist_tail = NULL;
}

int errs = 0;
void save_trace_file (void) {
  char ch,fname[10];
  FILE *src, *dest;
  struct stat st = {0};
  printf ("[SCHEDULER] Saving trace file...");
  errs++;
  if (stat("./errs", &st) == -1) {
    mkdir("./errs", 0755);
  }
  sprintf (fname, "errs/trace_%d",errs);  
  src = fopen ("trace","r");
  dest = fopen (fname,"w");
  while ((ch=fgetc(src)) != EOF)
    fputc (ch,dest);
  fclose (src);
  fclose (dest);
  printf ("done.\n");
}

int main (int argc, char *argv[])
{
  int done=0;
  int nos=0;
  struct timeval begin, end;
  
  gettimeofday(&begin, NULL);
  while (1) {
    if (access ("trace", F_OK) != -1) {
      // trace exists
      //printf ("trace exists. Not firstrun.\n");

      // Parse trace, populate the state variables in the linked list

      FILE *fp = fopen ("trace","r");
      char line_temp[256];
      int size = sizeof(line_temp);
      struct State_elem se;
      char func_name[32];
      int i=0,j=0;
      char *str1, *token, *saveptr1, delim=' ';
      struct Replay_list_elem *rle;

      while (fgets (line_temp, size, fp) != NULL) {
	rle = get_new_rle ();
	for (i=0, str1=line_temp; ; i++, str1=NULL) {
	  token = strtok_r(str1, &delim, &saveptr1);
	  if (token==NULL)
	    break;
	  switch (i) {
	  case 0:
	  case 1:
	  case 5:
	    {
	      break;
	    }
	  case 2:
	    {
	      se.cur_tid = atoi(token);
	      break;
	    }
	  case 3:
	    {
	      se.sync_pt = atoi(token);
	      break;
	    }
	  case 4:
	    {
	      se.choice = atoi(token);
	      add_se_to_rlist (rle, se);
	      break;
	    }
	  default:
	    {
	      if (token[0] != '\n') {
		add_enabled_thread (rle,atoi(token));
	      }
	      break;
	    }
	  }
	}
	j++;
      }
      fclose (fp);

      // Create a new state variable based on the old ones
      done = get_next_replay();
      free_replay_list();
      if (done == 1) {
	gettimeofday(&end, NULL);
	double time1 = (double)begin.tv_sec*1000 + (double)begin.tv_usec/1000;
	double time2 = (double)end.tv_sec*1000 + (double)end.tv_usec/1000;
	printf ("\n");
	printf ("[SCHEDULER] Total time taken = %lf ms\n",(time2-time1));
	printf ("[SCHEDULER] Schedules explored = %d\n",nos);
	printf ("[SCHEDULER] Buggy schedules found = %d\n",errs);	
	printf ("[SCHEDULER] End of testing. Have a nice day.\n");
	return 0;
      }
    }

    pid_t test_subject = fork();
    if (test_subject == 0) { // Child
      execv (argv[1], NULL);
    } else { // Parent
      int status;
      waitpid (-1, &status, 0);
      nos++;

      // Has trace been created?
      if (access ("trace", F_OK) != -1) {
	//printf ("trace created.\n");
      } else {
	printf ("[SCHEDULER] Error: tracefile has not been created.\n");
	return -1;
      }

      // Child has now exited.
      if (WIFEXITED (status)) {
	printf ("[SCHEDULER] Child exited normally.\n");
      } else {
	printf ("[SCHEDULER] Child encountered error(s).\n");
	save_trace_file ();
      }
    }
  }
}
