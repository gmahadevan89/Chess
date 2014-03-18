#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

int a=0,b=0;

void thread1 (void *args)
{
  a = 1;
  pthread_exit (NULL);
}

void thread2 (void *args)
{
  if (a == 1)
    b = 1;
  pthread_exit (NULL);
}

void thread3 (void *args)
{
  if (a == 1)
    b++;
  pthread_exit (NULL);
}

int main ()
{
  void **retval;
  pthread_t t1, t2, t3;
  pthread_create (&t1, NULL, (void*)thread1, NULL);
  pthread_create (&t2, NULL, (void*)thread2, NULL);
  pthread_create (&t3, NULL, (void*)thread3, NULL);
  pthread_join (t1, retval);
  assert (b != 2);
  pthread_join (t2, retval);
  pthread_join (t3, retval);
}
