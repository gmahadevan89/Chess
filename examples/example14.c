#include <stdio.h>
#include <pthread.h>
#include <assert.h>

int a=0;
int b=0;

void thread1 (void* args)
{
  if (a == 1)
    b = 1;
  pthread_exit (NULL);
}

void thread2 (void* args)
{
  if (b == 1)
    assert (a != 1);
  pthread_exit (NULL);
}

int main ()
{
  void **retval;
  pthread_t t1, t2;
  pthread_create (&t1, NULL, (void*)thread1, NULL);
  pthread_create (&t2, NULL, (void*)thread2, NULL);
  a = 1;
  pthread_join (t1, retval);
  pthread_join (t2, retval);
  printf ("a = %d b = %d\n",a,b);
}
