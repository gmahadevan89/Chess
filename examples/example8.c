#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>

int a=0;


void thread2(void *args)
{
  a++;
  pthread_exit(NULL);
}

void thread1(void *args)
{
    pthread_t t2;
    int s1=a;
    pthread_create(&t2,NULL,(void*)thread2,NULL);
    int s2=a;
    assert(s1==s2);
    pthread_exit(NULL);
}
void main()
{
    void **retval;
    pthread_t t1;
    pthread_create(&t1,NULL,(void*)thread1,NULL);
    pthread_join(t1,retval);
}
