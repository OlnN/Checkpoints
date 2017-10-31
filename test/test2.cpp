#include <atomic>
#include <thread>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <list>
#include <queue>
#include <vector>
#include <signal.h>
#include "checkpoint.h"

using namespace std;
checkpoint chkp;
#define NTHREAD 10

#define NWTHREAD 10
#define NRTHREAD 100
#define NOP 1000

const struct timespec ts{0, 10000};
int t = 10000;
struct timespec ts2;
atomic<bool> finish(false);

// This is a test with several readers and several writers accessing same vector of data.
// Checkpoints here are used instead of rwlock, condition variable and flags
// Several reader threads read_thread2() and several writer threads write_thread2() share access to data vector.
// Writer threads perform following operations in cycle:
// (write_readlock) Acquire reader lock, incrementing rlock flag. Value of rlock is a number of threads 
// (that currently are reading data vector. 
// (write_writeawait) One of writer threads sets wlock_await flag, and all other threads reduce rlock.
// (write_locked) The writer thread that set wlock_await flag then acquire writer lock by setting wlock flag. 
// (write_unlock) After adding data to the vector writer thread resets wlock flag.
// Reader threads acquire reader locks(read_lock) by incrementing rlock count if neither wlock nor wlock_await flags
// are set by writer threads or await till these flags become reset. 
// After reading data they unlock it(read_unlock) by reducing rlock count.


int rlock = 0;			// read lock count
int wlock = 0;			// 1 if write locked
int wlock_await = 0;		// 1 if write_thread2 awaits write lock but not write_locked yet

std::vector<int> data;
void write_thread2()
{
	int i=0;
	t+= rand()%1000;
	const struct timespec ts{0, 10000+t};
	for (int i=0;i<NOP;i++) {
		nanosleep(&ts, &ts2);
		chkp.pass([]()->int { return !wlock; }, []()->int { rlock++; return 0; }, "write_readlock");
		nanosleep(&ts, &ts2);
		int awaiting = chkp.pass([]()->int { return 1; },
			[]()->int {
				if (!wlock_await) {
					wlock_await = 1;
					return 1;
				} else {
					rlock--;
					return 0;
				}
			}, "write_writeawait");
		if (awaiting) {
			chkp.pass([]()->int { return rlock==1; },
					[]()->int { rlock = 0; wlock_await = 0; wlock = 1; return 0; }, "write_locked");
			data.push_back(i);
			chkp.pass([]()->int { return 1; }, []()->int { wlock = 0; return 0; }, "write_unlock");
		}
	}
}

void read_thread2()
{
	int i=0;
	t+= rand()%1000;
	const struct timespec ts{0, 10000+t};
	while (!finish) {
		int locked = chkp.pass([]()->int { return !wlock && !wlock_await; }, []() { rlock++; return 1; }, "read_lock");
		nanosleep(&ts, &ts2);
		if (data.size() > 0) {
			int v = data[random()%data.size()];
			fprintf(stderr, "%d_", v );
		}
		chkp.pass([]()->int { return 1; }, []()->int { rlock--; return 0; }, "read_unlock");
	}
}

void test2()
{
	thread t[NRTHREAD+NWTHREAD];
	for (int i=0; i<NRTHREAD; i++)
		t[i] = thread(read_thread2);
	for (int i=0; i<NWTHREAD; i++)
		t[NRTHREAD+i] = thread(write_thread2);
	for (int i=NRTHREAD; i<NRTHREAD+NWTHREAD; i++)
		t[i].join();
	finish = true;
	for (int i=0; i<NRTHREAD; i++)
		t[i].join();
	fprintf(stderr, "\nDone.\n");
}

int signal_exit = 0;
void signal_dump(int signal)
{
	signal_exit++;
	fprintf(stderr, "%s\n", chkp.print().c_str());
	if (signal_exit > 1) {
		fprintf(stderr, "\nExiting.\n");
		exit(0);
	}
}

int main()
{
	signal(SIGINT, signal_dump);             // Ctrl+C
	test2();
	return 0;
}
