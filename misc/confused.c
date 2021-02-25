/**
 * File: confused.c
 * Presents a pthreads example illustrating how data is (clumsily) shared with threads
 */

#include <stdio.h>
#include <pthread.h>

static const char *kFriends[] = {
	"Jack", "May", "Fukuya", "Rich", "Jordan", "Lisa",
	"Imaginary"
};
static const size_t kNumFriends = sizeof(kFriends)/sizeof(kFriends[0]) - 1;

static void *meetup(void *args) {
	const char *name = kFriends[*(size_t *)args];
	printf("Hey, I'm %s. Empowered to meet you.\n", name);
	return NULL;
}

int main(void) {
	printf("%zu friends meet.\n", kNumFriends);
	pthread_t friends[kNumFriends];
	for (size_t i = 0; i < kNumFriends; i++)
		pthread_create(&friends[i], NULL, meetup, &i);
	for (size_t j = 0; j < kNumFriends; j++)
		pthread_join(friends[j], NULL);
	printf("Is everyone eccounted for?\n");
	return 0;
}
