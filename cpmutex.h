#ifndef __CPMUTEX__
#define __CPMUTEX__

// Implementation of mutex
// Doesn't support double unlocking after single lock

#include <unistd.h>
#include <sys/syscall.h>
#include <bits/syscall.h>
#include <linux/futex.h>
#include <atomic>
#include <list>

class cpmutex {
public:
	cpmutex(): count(0), locked(0) {}
	void lock();
	void unlock();
private:
	std::atomic<int> count;
	std::atomic<int> locked;
};

#endif
