#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<pthread.h>

int bal = 0;
pthread_t t2;
pthread_mutex_t mut;

void depositor(void)
{
  printf("Inside depositor\n");
  pthread_mutex_lock(&mut);
  bal=bal+100;
  pthread_mutex_unlock(&mut);
  pthread_exit(&bal);

}

void withdrawer(void)
{
  printf("Inside Withdrawer\n");
  pthread_create(&t2,NULL,(void *)depositor,NULL);
  pthread_mutex_lock(&mut); 
  assert(bal!=0);
  bal=bal-100;
  pthread_mutex_unlock(&mut);
  pthread_exit(&bal);
}

void main()
{
  pthread_t t1;
  void **retval;
  pthread_mutex_lock(&mut);
  pthread_create(&t1,NULL,(void *)withdrawer,NULL);
  pthread_mutex_unlock(&mut);
  pthread_join(t1,retval);
  pthread_join(t2,retval);
  printf("Main exiting with balance= %d\n",bal);
}
