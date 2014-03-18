#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

pthread_mutex_t m1, m2;
int a=0,b=0;

void thread1 (void *args)
{
  pthread_mutex_lock (&m1);
  if (a == 1) {
    pthread_mutex_lock (&m2);
    b++;
  }
  pthread_mutex_unlock (&m1);
  if (b == 1)
    pthread_mutex_unlock (&m2);  
  pthread_mutex_lock (&m1);
  pthread_mutex_unlock (&m1);
  pthread_exit (NULL);
}

void thread2 (void *args)
{
  a = 1;
  pthread_mutex_lock (&m1);
  b--;
  pthread_mutex_lock (&m2);
  pthread_mutex_unlock (&m1);
  pthread_mutex_unlock (&m2);
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
