#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

int a=0,c=0,*b;
pthread_mutex_t m;

void thread1 (void *args)
{
  b = (int*) malloc (2*sizeof(int));
  pthread_mutex_lock (&m);
  a++;
  pthread_mutex_unlock (&m);
  b[1] = 42;
  pthread_exit (NULL);
}

void thread2 (void *args)
{
  pthread_mutex_lock (&m);
  if (a==1)
    c = 1;
  pthread_mutex_unlock (&m);
  pthread_exit (NULL);
}

int main ()
{
  void **retval;
  pthread_t t1, t2;
  pthread_create (&t1, NULL, (void*)thread1, NULL);
  pthread_create (&t2, NULL, (void*)thread2, NULL);
  if (c == 1)
    free (b);
  pthread_join (t1, retval);
  pthread_join (t2, retval);
}

