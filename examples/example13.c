#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>

//int init = 0;
int x=3;

void thread2(void *args)
{
    x=0;
    pthread_exit(NULL);

}

void thread1(void *args)
{
    pthread_t t2;
    if(x!=0)
    {    
        pthread_create(&t2,NULL,(void*)thread2,NULL);
        assert(x!=0);
    }
    pthread_exit(NULL);
}

void main()
{
    void **retval;
    pthread_t t1;
    pthread_create(&t1,NULL,(void*)thread1,NULL);
    pthread_join(t1,retval);
}
