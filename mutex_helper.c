#include <stdio.h>
#include <stdlib.h>
#include "mypthreadwrapper.h"
#include <assert.h>
extern struct Mutex_map_elem *mmap_head,*mmap_tail;

/*search for a given mutex variable in the map*/
struct Mutex_map_elem* search_mutex_map(pthread_mutex_t *mutex)
{
  struct Mutex_map_elem *q= mmap_head;
  while(q!=NULL)
    {
      if(mutex == q->mutex)
	{
	  return q;
	}
		
      q=q->next;
    }
	
  return NULL;

}

/*add a given mutex to the mutex map*/
void add_to_mutex_map(pthread_mutex_t *mutex,pthread_t holder)	
{
  struct Mutex_map_elem *temp=(struct Mutex_map_elem *)malloc(sizeof(struct Mutex_map_elem));
  assert(temp!=NULL);
  temp->mutex=mutex;
  temp->current_holder=holder;
  temp->waiters_head=temp->waiters_tail=NULL;
	
  if(mmap_head==NULL)
    {
      mmap_head=mmap_tail=temp;
    }
  else
    {
      mmap_tail->next=temp;
      mmap_tail=temp;
    }
}

/*add waiter to a given mutex if not available for locking*/
void add_waiter(struct Mutex_map_elem *mmap_elem , pthread_t waiter)
{
  //printf("Inside add_waiter with mut = %lu and thread = %lu\n",mmap_elem,waiter);
  struct Waiters_elem *temp = (struct Waiters_elem *)malloc(sizeof(struct Waiters_elem));
  assert(temp!=NULL);
  temp->tid=waiter;
  temp->next=NULL;
  //printf("Adding to waiters list %lu\n",waiter);
  if (mmap_elem->waiters_head==NULL)
    {
      mmap_elem->waiters_head=mmap_elem->waiters_tail=temp;
    }
  else
    {
      mmap_elem->waiters_tail->next=temp;
      mmap_elem->waiters_tail=temp;
    }
}
 
/*release waiters on given mutex*/
void release_waiters(struct Mutex_map_elem *mmap_elem)
{
  //printf("Inside release waiters\n");
  struct Waiters_elem *q=mmap_elem->waiters_head, *r;
  while(q!=NULL)
    {
      struct Thread_map *thread = search_thread_map(q->tid);
      assert(thread!=NULL);
      thread->status=ENABLED;
      //printf("Enabling thread:%lu\n",thread->actual_id);
      q=q->next;
    }
}
 
/*helper function to remove thread from waiters list of mutex*/
void remove_from_waiters(struct Mutex_map_elem *mut,pthread_t thread)
{
  //printf("Inside remove_from_waiters with mut = %lu and thread = %lu\n",mut,thread);
  struct Waiters_elem *prev=NULL,*q= mut->waiters_head;
  assert(q!=NULL);
  while(q!=NULL)
    {
      if(thread==q->tid)
	{
	  //printf("removed successfully\n");
	  if(prev==NULL)
	    {
	      mut->waiters_head=q->next;
	      if(q->next==NULL)
                {
		  mut->waiters_tail=NULL;
                }                 
	      free(q);
	      break;
	    }
	  else
	    {
	      prev->next=q->next;
	      free(q);
	      break;
	    }

	}

      prev=q;
      q=q->next;
    }
}
     
/*helper to remove a mutex from map*/
void remove_from_mutex_map(struct Mutex_map_elem *mut)
{

  struct Mutex_map_elem *prev=NULL,*q= mmap_head;
  while(q!=NULL)
    {
      if(q->mutex==mut->mutex)
	{
	  if(prev==NULL)
	    {
	      mmap_head=q->next;
	      free(q);
	      break;
	    }
	  else
	    {
	      prev->next=q->next;
	      free(q);
	      break;
	    }

	}

      prev=q;
      q=q->next;
    }	        

}
