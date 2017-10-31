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

// This is a test with many threads work with same variable.
// Checkpoints here are used instead of mutex
// (thread_lock) and (lock) are equivalent to locking a mutex.
// (thread_unlock) and (unlock) are equivalent to unlocking it.


checkpoint chkp;

#define NTHREAD 10

const struct timespec ts{0, 10000};
int t = 10000;
struct timespec ts2;

int locked = 0;

void thread1()
{
	char sid[100];
	sprintf(sid, "%d", this_thread::get_id());
	t+= rand()%10000;
	const struct timespec ts{0, 10000000+t};

	string name1 = "thread_lock["; name1+=sid;name1+="]";
	chkp.pass([]()->int { return !locked; }, []()->int { locked = 1; return 0; }, name1);

	nanosleep(&ts, &ts2);

	string name2 = "thread_unlock["; name2+=sid;name2+= "]";
	chkp.pass([]()->int { return 1; }, []()->int { locked = 0; return 0; }, name2);
}

int test1()
{
	char sid[100];
	sprintf(sid, "%d", this_thread::get_id());
	string name1 = "lock[";name1+=sid;name1+= "]";

	chkp.pass([]()->int {return !locked;}, []()->int{locked = 1; return 0;}, name1);

	thread t[NTHREAD];
	for (int i=0; i<NTHREAD; i++)
		t[i] = thread(thread1);
	const struct timespec ts3{0, 100000};
	nanosleep(&ts3, &ts2);


	string name2 = "unlock[";name2+=sid;name2+= "]";
	chkp.pass([]()->int {return 1;}, []()->int{locked = 0; return 0;}, name2);

	for (int i=0; i<NTHREAD; i++)
		t[i].join();
	cerr << "finish.\n";
	return 0;
}

int signal_exit = 0;
void signal_dump(int signal)
{
	signal_exit++;
	cerr << chkp.print() << endl;
	if (signal_exit > 1) {
		cerr <<"\nExiting.\n";
		exit(0);
	}
}

int main()
{
	signal(SIGINT, signal_dump);             // Ctrl+C
	test1();
	return 0;
}
