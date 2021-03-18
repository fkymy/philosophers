#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct params {
	int id;
	sem_t *s;
} t_params;

void *writer(void *args)
{
	t_params *p;

	p = (t_params *)args;
	printf("Sending signal %d\n", p->id);
	sem_post(p->s);
	return NULL;
}

void *read_after_ten(void *args)
{
	sem_t *s;

	s = (sem_t *)args;
	sem_wait(s);
	printf("Starting after 10!\n");
	return NULL;
}

int main(void)
{
	t_params params[10];
	pthread_t thread_writers[10];
	pthread_t thread_reader;
	sem_t *neg_sem;

	neg_sem = sem_open("/neg_sem", O_CREAT|O_EXCL, S_IRWXU, -9);
	sem_unlink("/neg_sem");
	for (int i = 0; i < 10; i++)
	{
		params[i].id = i;
		params[i].s = neg_sem;
		pthread_create(&thread_writers[i], NULL, writer, (void *)&params[i]);
	}
	pthread_create(&thread_reader, NULL, read_after_ten, (void *)neg_sem);
	for (int j = 0; j < 10; j++)
		pthread_join(thread_writers[j], NULL);
	sem_close(neg_sem);
}
