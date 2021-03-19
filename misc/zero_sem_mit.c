#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

void *thread_A(void *arg)
{
	sem_t *s = arg;
	printf("A1\n");
	printf("A2\n");
	sem_post(s);
	printf("A3\n");
	printf("A4\n");
	return NULL;
}

void *thread_B(void *arg)
{
	sem_t *s = arg;
	printf("B1\n");
	printf("B2\n");
	printf("B3\n");
	sem_wait(s);
	printf("B4\n");
	return NULL;
}

int main(void)
{
	sem_t *s;
	pthread_t tid_A;
	pthread_t tid_B;

	s = sem_open("/zero_sem", O_CREAT|O_EXCL, S_IRWXU, 0);
	sem_unlink("/zero_sem");
	pthread_create(&tid_A, NULL, thread_A, (void *)s);
	pthread_create(&tid_B, NULL, thread_B, (void *)s);
	pthread_join(tid_A, NULL);
	pthread_join(tid_B, NULL);
	sem_close(s);
	return 0;
}
