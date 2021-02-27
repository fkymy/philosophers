#include <thread>
#include <iostream>
#include <cstdlib>

using namespace std;

static void greet(size_t id) {
	for (size_t i = 0; i < id; i++) {
		printf("Greeter #%zu says Hello!\n", id);
	}
	struct timespec ts = {
		0, random() % 1000000000
	};
	nanosleep(&ts, NULL);
	printf("Greeter #%zu has issued all of their hellos, so they go home!\n", id);
}

static const size_t kNumGreeters = 6;
int main(int argc, char *argv[]) {
	cout << "Welcome to Greetland!" << endl;
	thread greeters[kNumGreeters];
	for (size_t i = 0; i < kNumGreeters; i++) {
		greeters[i] = thread(greet, i + 1);
	}
	for (thread &greeter : greeters) {
		greeter.join();
	}
	cout << "Everyone's all greetedd out!" << endl;
}
