#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

void *produce(void *args)
{
	sem_t *s = args;
	for (int i = 0; i < 5; i++)
	{
		printf("Now producing %d\n", i);
		sem_post(s);
	}
	return NULL;
}

void *consume(void *args)
{
	sem_t *s = args;
	for (int i = 0; i < 5; i++)
	{
		sem_wait(s);
		printf("Now consuming %d\n", i);
	}
	return NULL;
}

int main(void)
{
	pthread_t thread_producer;
	pthread_t thread_consumer;
	sem_t *zero_sem;

	zero_sem = sem_open("/zero_sem", O_CREAT|O_EXCL, S_IRWXU, 0);
	sem_unlink("/zero_sem");
	pthread_create(&thread_producer, NULL, produce, (void *)zero_sem);
	pthread_create(&thread_consumer, NULL, consume, (void *)zero_sem);
	pthread_join(thread_producer, NULL);
	pthread_join(thread_consumer, NULL);
	sem_close(zero_sem);
	return 0;
}
