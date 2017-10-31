#include <atomic>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <list>
#include <queue>
#include <vector>
#include <signal.h>
#include "checkpoint.h"

using namespace std;
#include <stdio.h>

// This is a test for a cpmutex implementation of mutexes

#define NTHREAD 10
#define NOP 10000
int N;
cpmutex m;

void thread_cpmutex()
{
	for (int i=0; i<NOP; i++) {
		m.lock();
		N++;
		m.unlock();
	}
}

void test_cpmutex()
{
	thread t[NTHREAD];
	for (int i=0; i<NTHREAD; i++)
		t[i] = thread(thread_cpmutex);
	const struct timespec ts3{0, 100000};
	//	nanosleep(&ts3, &ts2);

	for (int i=0; i<NTHREAD; i++)
		t[i].join();
	cerr << "N="<< N << endl;
	cerr << "Correct N="<< NOP*NTHREAD << endl;
	cerr << "finish.\n";
}

int main()
{
	test_cpmutex();
	return 0;
}
