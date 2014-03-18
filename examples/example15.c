#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

pthread_mutex_t m;
int *a;
int b=0;

void thread1 (void* args)
{
  pthread_mutex_lock (&m);
  b = 1;
  pthread_mutex_unlock (&m);
  a = (int*) malloc (5*sizeof(int));
  pthread_exit (NULL);
}

void thread2 (void* args)
{
  pthread_mutex_lock (&m);
  if (b == 1)
    a[4] = 42;
  pthread_mutex_unlock (&m);
  pthread_exit (NULL);
}

int main ()
{
  void **retval;
  pthread_t t1, t2;
  pthread_create (&t1, NULL, (void*)thread1, NULL);
  pthread_create (&t2, NULL, (void*)thread2, NULL);
  pthread_join (t1, retval);
  pthread_join (t2, retval);
}
