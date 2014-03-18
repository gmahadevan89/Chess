#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

pthread_mutex_t l;

void func (void *args)
{
    pthread_mutex_lock(&l);
    printf("Inside func after acquiring mutex\n");
    pthread_mutex_unlock(&l);
    pthread_exit(NULL);
}

void main()
{
    pthread_mutex_lock(&l);
    void **retval;
    printf("Inside main after acquiring mutex\n");
    pthread_t t1;
    pthread_create(&t1,NULL,(void *)func,NULL);
    pthread_mutex_unlock(&l);
    printf("Inside main after releasing lock\n");
    pthread_join(t1,retval);
}
