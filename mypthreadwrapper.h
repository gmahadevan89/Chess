#ifndef __MYPTHREADWRAPPER_H
#define __MYPTHREADWRAPPER_H


#define ENABLED 111
#define EXITED	222
#define WAITING_TO_JOIN 333
#define WAITING_FOR_LOCK 444

void add_to_thread_map(pthread_t);
struct Thread_map * search_thread_map(pthread_t);

static void initialize_original_functions();

struct Thread_Arg{
	void* (*func)(void *);
	void* arg;
};

struct Thread_map{
	pthread_t actual_id;
	int tid;
	int status;
	pthread_t to_join; 	
	int sync_pts;
	struct Thread_map *next;
};

struct State_elem{
	int cur_tid;
	int sync_pt;
	int choice;
	struct State_elem *prev;
	struct State_elem *next;
};

struct Waiters_elem{
	pthread_t tid;
	struct Waiters_elem *next;
};


struct Mutex_map_elem{
	pthread_t current_holder;
	pthread_mutex_t *mutex;
	struct Waiters_elem *waiters_head,*waiters_tail;
	struct Mutex_map_elem *next;
};

struct Thread_ids{

	int value;
	struct Thread_ids *next;
};

#endif
