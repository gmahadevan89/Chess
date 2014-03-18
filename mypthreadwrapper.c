#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>
#include<stdbool.h>
#include<assert.h>
#include<unistd.h>
#include "mypthreadwrapper.h"
#include "mutex_helper.h"

bool initialized=false,record=false;
int current_tid=0;
int scheduled_tid=0;
int thread_count=0;
FILE *fp = NULL;
FILE *rp;

struct Thread_map *tmap_head=NULL,*tmap_tail=NULL;
struct State_elem *selem_head=NULL,*selem_tail=NULL;
struct Mutex_map_elem *mmap_head=NULL,*mmap_tail=NULL;
struct Thread_ids *tid_current=NULL,*tid_tail=NULL;

int (*original_pthread_create)(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) = NULL;
int (*original_pthread_join)(pthread_t, void**) = NULL;
int (*original_pthread_mutex_lock)(pthread_mutex_t*) = NULL;
int (*original_pthread_mutex_unlock)(pthread_mutex_t*) = NULL;
int (*original_pthread_cond_wait)(pthread_cond_t *,pthread_mutex_t *)=NULL;
pthread_t (*original_pthread_self)(void)=NULL;
int (*original_pthread_mutex_trylock)(pthread_mutex_t *)=NULL;
int (*original_pthread_equal)(pthread_t,pthread_t)=NULL;
int (*original_pthread_cond_broadcast)(pthread_cond_t*)=NULL;
int (*original_pthread_tryjoin_np)(pthread_t , void **)=NULL;
void (*original_pthread_exit)(void *)=NULL;

/*helper function to fetch next thread from Thread_ids list*/
int fetch_next_thread()
{
    if(tid_current->next==NULL)
    {
        record=true;
        return tid_current->value;
    }
    int ret = tid_current->value;
    struct Thread_ids *r=tid_current;
    tid_current=tid_current->next;
    free(r);
    return ret;
}



/*helper function for determining next thread to be scheduled*/ 
void determine_next_thread(int caller_tid)
{

    if(!record)
    {
        /*fetch thread id from struct of thread-ids*/		
        /*change scheduled_tid to new thread id*/
        scheduled_tid=fetch_next_thread();

    }	
    else
    {
        /*iterate through Thread_map and choose the first enabled thread except myself*/
        struct Thread_map *q= tmap_head;
        while(q!=NULL)
        {
            if(q->status==ENABLED)
            {
                scheduled_tid=q->tid;
                break;
            }
            q=q->next;
        }

        if(q==NULL)
        {
            /*Declare deadlock and exit*/
            printf("[MYPTHREADWRAPPER] Error: Deadlock encountered. Terminating program.\n");
            assert(0);
        }

    }

}


/*helper function to add a thread to thread_map*/
void add_to_thread_map(pthread_t thread)
{
    struct Thread_map *tm=(struct Thread_map *)malloc(sizeof(struct Thread_map ));
    assert(tm!=NULL);
    tm->actual_id=thread;
    tm->tid=thread_count++;
    tm->status=ENABLED;
    tm->to_join=(pthread_t)-1;
    tm->next=NULL;
    tm->sync_pts=1;
    tmap_tail->next=tm;
    tmap_tail=tm;
}

/*helper function to add a state element to state list*/
void add_to_state(int cur_tid,int sync_pt,int choice)
{
    struct State_elem *temp = (struct State_elem *)malloc(sizeof (struct State_elem));
    assert(temp!=NULL);
    temp->cur_tid=cur_tid;
    temp->sync_pt = sync_pt;
    temp->choice=choice;
    temp->prev=selem_tail;
    temp->next=NULL;
    selem_tail->next=temp;
    selem_tail=temp;
}

/*helper function to print to file at a sync point*/
void print_to_file(struct Thread_map *myid,char *op)
{
    /* printf ("Printing to trace\n"); */
    fp = fopen ("trace","a");
    fprintf(fp,"%s ",op);
    struct State_elem *q = selem_tail;
    while(q!=NULL)
    {
        fprintf(fp,"* %d %d %d ",q->cur_tid,q->sync_pt,q->choice);
        q=q->next;
    }
    fprintf(fp,"# ");
    struct Thread_map *tm=tmap_head;
    while(tm!=NULL)
    {
        if(tm->status==ENABLED)
            fprintf(fp,"%d ",tm->tid);
        tm=tm->next;
    }
    fprintf(fp,"\n");
    fclose (fp);
}


/*helper function to search tid of a thread*/
struct Thread_map* search_thread_map(pthread_t thread)
{
    //printf("Searching for %lu\n",thread);
    struct Thread_map *q=tmap_head;
    while(q!=NULL)
    {
        //printf("q->actual_id = %lu , thread = %lu\n",q->actual_id,thread); 	
        if(original_pthread_equal (thread, q->actual_id) !=0 )
        {
            //printf("Successful Search\n");
            return q;
        }
        q=q->next;
    }
    return NULL;
}

/*helper function to remove a thread from thread map*/
void remove_from_thread_map(pthread_t thread)
{
    struct Thread_map *prev=NULL,*q=tmap_head;
    while(q!=NULL)
    {
        if(pthread_equal(thread,q->actual_id)!=0)
        {
            if(prev==NULL)
            {
                tmap_head=q->next;
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

pthread_mutex_t mutex,create_thread;
pthread_cond_t cond,spawn;

/*Wrapper function for calling a thread's start routine after acquiring mutex*/
static void* thread_main(void* arg)
{
    pthread_t test;
    test=original_pthread_self();
    //printf("pthread_self id = %lu\n",test);
    add_to_thread_map(test);
    struct Thread_map *myself = search_thread_map(original_pthread_self());
    assert(myself!=NULL);
    original_pthread_cond_broadcast(&spawn);
    original_pthread_mutex_lock(&mutex);
    original_pthread_cond_broadcast(&cond);
    while(myself->tid!=scheduled_tid)
    {
        original_pthread_cond_wait(&cond,&mutex);
    }
    struct Thread_Arg thread_arg =*(struct Thread_Arg *)arg;
    free(arg);
    thread_arg.func(thread_arg.arg);
}

/*Wrapper for pthread_create*/ 
int pthread_create(pthread_t *thread,pthread_attr_t *attr,void* (*start_routine)(void*),void* args)
{
    //printf("INSIDE PTHREAD_CREATE:\n\n"); 
    if (!initialized)
    {
        initialize_original_functions(); 
    }


    struct Thread_Arg *thread_arg=(struct Thread_Arg *)malloc(sizeof(struct Thread_Arg ));
    assert(thread_arg!=NULL);
    thread_arg->func=start_routine;
    thread_arg->arg=args;
    int k= original_pthread_create(thread, attr ,thread_main , thread_arg);
    original_pthread_mutex_lock(&create_thread);
    original_pthread_cond_wait(&spawn,&create_thread);
    original_pthread_mutex_unlock(&create_thread);
    //printf ("pthread_create: thread = %lu\n",*thread);
    struct Thread_map *myid = search_thread_map(original_pthread_self());
    assert(myid!=NULL);
    /*determine next thread to be scheduled from the scheduler
      and assign that id to scheduled_id*/
    if(!record)
    {
        determine_next_thread(myid->tid);
    }
    /*print to file scheduled_tid */
    add_to_state(myid->tid,myid->sync_pts++,scheduled_tid);
    print_to_file(myid,"pthread_create");
    while(myid->tid!=scheduled_tid)
    {
        original_pthread_cond_wait(&cond,&mutex);
    } 

    return k;

}

/*Wrapper for pthread_join*/
int pthread_join(pthread_t thread,void **retval)
{
    //printf("INSIDE PTHREAD_JOIN\n\n");
    if(!initialized)
    {
        initialize_original_functions();
    }

    struct Thread_map *myself = search_thread_map(original_pthread_self());
    assert(myself!=NULL);
    struct Thread_map *joining_thread=search_thread_map(thread);
    assert(joining_thread!=NULL);
    /*add myself to exiting thread for notification*/
    joining_thread->to_join=myself->actual_id;
    myself->status=WAITING_TO_JOIN;	
    while(joining_thread->status!=EXITED)	
    {	
        determine_next_thread(myself->tid);

        add_to_state(myself->tid,myself->sync_pts,scheduled_tid);
        print_to_file(myself,"pthread_join");
        original_pthread_cond_broadcast(&cond);
        while(myself->tid!=scheduled_tid)
        {
            original_pthread_cond_wait(&cond,&mutex);
        }
    }

    myself->status=ENABLED;
    myself->sync_pts++;
    while(original_pthread_tryjoin_np(thread,retval)!=0);
    //printf("Thread %d joined\n",joining_thread->tid);
    remove_from_thread_map(thread);
    return 0;


}

/*Wrapper for pthread_exit*/
void pthread_exit(void *retval)
{
    //printf("INSIDE PTHREAD_EXIT\n\n");
    struct Thread_map *myid = search_thread_map(original_pthread_self());
    assert(myid!=NULL);
    myid->status=EXITED;
    if(myid->to_join!=-1)
    {
        struct Thread_map *jointhread=search_thread_map(myid->to_join);
        jointhread->status=ENABLED;
    }
    original_pthread_cond_broadcast(&cond);
    determine_next_thread(myid->tid);
    //printf("After determining next thread in exit: %d\n",scheduled_tid);
    add_to_state(myid->tid,myid->sync_pts++,scheduled_tid);
    print_to_file(myid,"pthread_exit");	
    original_pthread_mutex_unlock(&mutex);
    //printf("Before Calling original_pthread_exit\n");
    original_pthread_exit(retval); 
}

/*Wrapper for pthread_mutex_lock*/
int pthread_mutex_lock(pthread_mutex_t *mut)
{
    //printf("INSIDE PTHREAD_MUTEX_LOCK\n\n");
    if(!initialized)
    {
        initialize_original_functions();
    }
    struct Thread_map *myself=search_thread_map(original_pthread_self());
    //printf("LOCK ATTEMPTED BY THREAD:%lu\n",myself->actual_id);
    if(myself->status==EXITED) {
        return original_pthread_mutex_lock(mut);
    }
    struct Mutex_map_elem *lock=search_mutex_map(mut);
    if(original_pthread_mutex_trylock(mut)==0)
    {
        if (lock==NULL)
        {
            add_to_mutex_map(mut,original_pthread_self());
        }
        else
        {

            remove_from_waiters(lock,myself->actual_id);
            /*current holder of lock is me*/
            lock->current_holder=myself->actual_id;
            /*change status of all other waiting threads if any to waiting for lock*/
            struct Waiters_elem *q=lock->waiters_head; 
            while(q!=NULL)
            {
                struct Thread_map *thread=search_thread_map(q->tid);
                assert(thread!=NULL);
                thread->status=WAITING_FOR_LOCK;
                q=q->next;
            }
        }
        determine_next_thread(myself->tid);
        add_to_state(myself->tid,myself->sync_pts++,scheduled_tid);
        print_to_file(myself,"pthread_mutex_lock");
        original_pthread_cond_broadcast(&cond);
        while(myself->tid!=scheduled_tid)
        {
            original_pthread_cond_wait(&cond,&mutex);
        }

        return 0;
    }
    else
    {
        assert(myself!=NULL);
        myself->status=WAITING_FOR_LOCK;
        add_waiter(lock,myself->actual_id);
        determine_next_thread(myself->tid);
        add_to_state(myself->tid,myself->sync_pts++,scheduled_tid);
        print_to_file(myself,"pthread_mutex_lock");
        original_pthread_cond_broadcast(&cond);
        while(myself->tid!=scheduled_tid)
        {
            original_pthread_cond_wait(&cond,&mutex);
        }
        //printf("BACK TO LOCKING AFTER WAITING: %lu\n",myself->actual_id);
        /*remove myself from waiting list*/
        remove_from_waiters(lock,myself->actual_id);
        /*current holder of lock is me*/
        lock->current_holder=myself->actual_id;
        /*change status of all other waiting threads if any to waiting for lock*/
        struct Waiters_elem *q=lock->waiters_head; 
        while(q!=NULL)
        {
            struct Thread_map *thread=search_thread_map(q->tid);
            assert(thread!=NULL);
            thread->status=WAITING_FOR_LOCK;
            q=q->next;
        }
        return(original_pthread_mutex_lock(mut));
    }
}

/*Wrapper for pthread_mutex_unlock*/
int pthread_mutex_unlock(pthread_mutex_t *mut)
{
    //printf("INSIDE PTHREAD_MUTEX_UNLOCK\n\n");
    struct Thread_map *myself=search_thread_map(original_pthread_self());
    if(myself->status==EXITED)
    {
        return original_pthread_mutex_unlock(mut);
    }
    struct Mutex_map_elem *lock = search_mutex_map(mut);
    assert(lock!=NULL);
    /*set status of all waiters to enabled*/
    //printf("UNLOCK ATTEMPTED BY THREAD:%lu\n",myself->actual_id);
    struct Waiters_elem *q= lock->waiters_head;
    if(q==NULL)
    {
        /*remove lock from mutex map*/
        remove_from_mutex_map(lock);      
    }
    else
    {
        release_waiters(lock);
    }   
    determine_next_thread(myself->tid);
    add_to_state(myself->tid,myself->sync_pts++,scheduled_tid);
    print_to_file(myself,"pthread_mutex_unlock");
    original_pthread_cond_broadcast(&cond);
    int k = original_pthread_mutex_unlock(mut);
    while(myself->tid!=scheduled_tid)
    {
        original_pthread_cond_wait(&cond,&mutex);
    }

    return k;

}




/*Initialize original functions*/
static void initialize_original_functions()
{

    original_pthread_self=(pthread_t (*)(void))dlsym(RTLD_NEXT,"pthread_self");
    original_pthread_create = (int (*)(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*))dlsym(RTLD_NEXT, "pthread_create");
    original_pthread_join =  (int (*)(pthread_t, void**))dlsym(RTLD_NEXT, "pthread_join");
    original_pthread_tryjoin_np=(int (*)(pthread_t,void **))dlsym(RTLD_NEXT,"pthread_tryjoin_np");
    original_pthread_mutex_lock = (int (*)(pthread_mutex_t*))dlsym(RTLD_NEXT, "pthread_mutex_lock");
    original_pthread_mutex_trylock=(int (*)(pthread_mutex_t*))dlsym(RTLD_NEXT,"pthread_mutex_trylock");
    original_pthread_mutex_unlock =(int (*)(pthread_mutex_t*))dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    original_pthread_equal=(int (*)(pthread_t,pthread_t))dlsym(RTLD_NEXT,"pthread_equal");
    original_pthread_cond_wait=(int (*)(pthread_cond_t *,pthread_mutex_t *))dlsym(RTLD_NEXT,"pthread_cond_wait");
    original_pthread_cond_broadcast=(int (*)(pthread_cond_t *))dlsym(RTLD_NEXT,"pthread_cond_broadcast");
    original_pthread_exit=(void (*)(void *))dlsym(RTLD_NEXT,"pthread_exit");


    /*acquire lock for the main thread*/
    original_pthread_mutex_lock(&mutex);

    /*put main in list of enabled threads and give it a tid of zero*/
    struct Thread_map *tm = (struct Thread_map *)malloc(sizeof(struct Thread_map ));
    assert(tm!=NULL);
    tm->actual_id = original_pthread_self();
    tm->tid=thread_count++;
    tm->next=NULL;
    tm->status=ENABLED;
    tm->sync_pts=1;
    tm->to_join=(pthread_t)-1;
    tmap_head=tmap_tail=tm;
    initialized=true;

    /*check whether trace file exists already*/
    if(access("replay",F_OK)!=-1)
    {
        rp=fopen("replay","r");
        int val;
        /*read from file and create Thread_ids list*/
        while(fscanf(rp,"%d",&val)!=EOF)
        {
            struct Thread_ids *q=(struct Thread_ids *)malloc(sizeof(struct Thread_ids));
            q->value=val;
            q->next=NULL;
            if(tid_current==NULL)
            {
                tid_current=tid_tail=q;
            }
            else
            {
                tid_tail->next=q;
                tid_tail=q;
            }
        }

        /*close file*/
        fclose(rp);

    }	
    else
    {
        record=true;
    }

    /*open file in write mode, erase its previous contents*/
    fp=fopen("trace","w");
    fclose (fp);

    /*Create start state*/
    struct State_elem *temp=(struct State_elem *)malloc(sizeof(struct State_elem));
    assert(temp!=NULL);
    temp->cur_tid=0;
    temp->sync_pt=0;
    temp->choice=0;
    temp->prev=NULL;
    temp->next=NULL;
    selem_head=selem_tail=temp;
}










