#include <iostream>
#include <thread>

using namespace std;

static void recharge() {
	printf("I recharge by spending time alone.\n");
}

static const size_t kNumIntroverts = 6;
int main(int argc, char *argv[]) {
	cout << "Let's hear from " << kNumIntroverts << " introverts." << endl;
	thread introverts[kNumIntroverts];
	for (thread &introvert : introverts) {
		introvert = thread(recharge);
	}
	for (thread &introvert : introverts) {
		introvert.join();
	}
	cout << "Everyone's recharged!" << endl;
	return 0;
}
