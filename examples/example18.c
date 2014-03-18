#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

int a=0,*b;
pthread_mutex_t m;

void thread1 (void *args)
{
  a = 1;
  pthread_mutex_lock (&m);
  b[1] = 25;
  pthread_mutex_unlock (&m);
  a = 0;
  pthread_exit (NULL);
}

void thread2 (void *args)
{
  pthread_mutex_lock (&m);
  if (a == 1)
    b[0] = 4;
  pthread_mutex_unlock (&m);
  pthread_exit (NULL);
}

int main ()
{
  void **retval;
  pthread_t t1, t2;
  pthread_create (&t1, NULL, (void*)thread1, NULL);
  pthread_create (&t2, NULL, (void*)thread2, NULL);
  pthread_mutex_lock (&m);
  b = (int*)malloc (2*sizeof(int));
  pthread_mutex_unlock (&m);
  pthread_join (t1, retval);
  pthread_join (t2, retval);
}
