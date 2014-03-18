#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>

int init = 0;
int *t=NULL;

void thread2(void *args)
{
    if(init)
    {
        assert(t!=NULL);
    }
    pthread_exit(NULL);

}

void thread1(void *args)
{
    pthread_t t2;
    init=1;
    pthread_create(&t2,NULL,(void*)thread2,NULL);
    t=(int*)malloc(sizeof(int));
    pthread_exit(NULL);
}

void main()
{
    void **retval;
    pthread_t t1;
    pthread_create(&t1,NULL,(void*)thread1,NULL);
    pthread_join(t1,retval);
}
