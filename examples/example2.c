#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

int bal = 0;

void depositor(void)
{
	printf("Inside depositor\n");
	bal=bal+100;
	pthread_exit(&bal);
}

void withdrawer(void)
{
	printf("Inside Withdrawer\n");
	assert(bal!=0);
	bal=bal-100;
	pthread_exit(&bal);
}

void main()
{
	pthread_t t1,t2;
	void **retval;
	pthread_create(&t1,NULL,(void *)withdrawer,NULL);
	pthread_create(&t2,NULL,(void *)depositor,NULL);
	pthread_join(t2,retval);
	pthread_join(t1,retval);
	printf("Main exiting with balance= %d\n",bal);
}
