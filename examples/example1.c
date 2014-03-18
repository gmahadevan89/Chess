#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>

int balance=100;
void depositor(void)
{
 printf("Inside depositor:\n");
 balance=balance+100;
 pthread_exit(&balance);
} 

void main()
{
 pthread_t t1;
 printf("Calling pthread_create for depositor thread: \n");
 pthread_create(&t1,NULL,(void *)depositor,NULL);
 pthread_join(t1,NULL);
 printf("Value of balance: %d\n",balance);
} 
