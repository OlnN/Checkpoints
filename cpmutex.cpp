#include "cpmutex.h"
using namespace std;

void cpmutex::lock()
{
	bool c = false;
	int z = 0;
	count++; // increase counter of waiting threads
	// atomically get value of "locked", and set it to 1(futex locked) if value was 0(futex was unlocked)
	c = locked.compare_exchange_strong(z, 1); // c == true if exchanged performed, false otherwise
	while (!c) {
		//this executes if mutex was locked(c==false - value of locked was not updated at line 10 or 16)
		// waits on futex until "locked" is changed and futex called with FUTEX_WAKE
		syscall(SYS_futex, &locked, FUTEX_WAIT, 1, 0, 0, 0, -1); 
		z = 0;
		c = locked.compare_exchange_strong(z, 1);
	}
}

void cpmutex::unlock()
{
	locked = 0; // unlocks futex variable
	if (count-- != 1) // if some threads are waiting
		syscall(SYS_futex, &locked, FUTEX_WAKE, 1, 0, 0, 0, -1); // wakes one waiting thread
}
