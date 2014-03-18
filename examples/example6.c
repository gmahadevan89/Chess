#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

pthread_mutex_t mut1,mut2;

void func(void *args)
{
    pthread_mutex_lock(&mut2);
    pthread_mutex_lock(&mut1);
    pthread_mutex_unlock(&mut2);
    pthread_mutex_unlock(&mut1);
    pthread_exit(NULL);
}

void main()
{
  pthread_t t1;
  pthread_create(&t1,NULL,(void*)func,NULL);
  pthread_mutex_lock(&mut1);
  pthread_mutex_lock(&mut2);
  pthread_mutex_unlock(&mut1);
  pthread_mutex_unlock(&mut2);
}


