#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<pthread.h>

int a=0,b=0;

void thread1 (void *args)
{
    int s1=a;
    int s2=a;
    int s3=b;
    int s4=b;
    assert((s1==s3) && (s2==s4) );
    pthread_exit(NULL);
}

void thread2 (void *args)
{
    a++;
    pthread_exit(NULL);
}

void thread3 (void *args)
{
    b++;
    pthread_exit(NULL);
}


void main ()
{
    void **retval=NULL;
    pthread_t t1,t2,t3;
    pthread_create(&t1,NULL,(void*)thread1,NULL);
    pthread_create(&t2,NULL,(void*)thread2,NULL);
    pthread_create(&t3,NULL,(void*)thread3,NULL);
    pthread_join(t1,retval);
    pthread_join(t2,retval);
    pthread_join(t3,retval);
}
       
