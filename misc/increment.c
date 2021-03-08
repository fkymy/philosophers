#include <stdio.h>
#include <pthread.h>

static const size_t kNumThreads = 4;

static void *increment_routine(void *args)
{
	int *np;

	np = (int *)args;
	for (int i = 0; i < 1000000; i++)
		*np += 1;
	return NULL;
}

int main(void)
{
	pthread_t threads[kNumThreads];
	int n = 0;

	for (size_t i = 0; i < kNumThreads; i++)
		pthread_create(&threads[i], NULL, increment_routine, &n);
	for (size_t j = 0; j < kNumThreads; j++)
		pthread_join(threads[j], NULL);
	printf("%d\n", n);
	return 0;
}
