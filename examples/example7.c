#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>

int a=0;

void thread1(void *args)
{
    assert(a==0);
    pthread_exit(NULL);
}

void thread2(void *args)
{
  a++;
  pthread_exit(NULL);
}

void main()
{
    void **retval;
    pthread_t t1,t2;
    pthread_create(&t1,NULL,(void*)thread1,NULL);
    pthread_create(&t2,NULL,(void*)thread2,NULL);
    pthread_join(t1,retval);
    pthread_join(t2,retval);
}
