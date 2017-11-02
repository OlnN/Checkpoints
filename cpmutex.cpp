#include "cpmutex.h"
using namespace std;

void cpmutex::lock()
{
	bool c = false;
	int z = 0;
	count++; // increase counter of waiting threads
	// atomically get value of "locked", and set it to 1(futex locked) if value was 0(futex was unlocked)
	// locking must be done atomically in case other threads are locking mutex at the same time too
	c = locked.compare_exchange_strong(z, 1); // c == true if exchanged performed, false otherwise
	while (!c) {
		// this is executed if mutex was locked(c==false - value of locked was not)
		// if kernel sees that *locked == 3rd argument(=1) of SYS_futex, then thread is enqueued in wait queue inside kernel
		// syscall waits on futex until futex called with FUTEX_WAKE
		// address of "locked" is id of mutex inside SYS_futex
		syscall(SYS_futex, &locked, FUTEX_WAIT, 1, 0, 0, 0, -1); // see futex.c in linux kernel source for detailed description
		// thread is waked from futex here; it tries to lock futex again
		// locking must be done atomically in case other threads are locking mutex at the same time too
		z = 0;
		c = locked.compare_exchange_strong(z, 1);
	}
}

void cpmutex::unlock()
{
	locked = 0; // marks futex variable unlocked
	if (count-- != 1) // if some other threads are waiting
		syscall(SYS_futex, &locked, FUTEX_WAKE, 1, 0, 0, 0, -1); // wakes one waiting thread
}
