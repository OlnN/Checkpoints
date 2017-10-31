#include "cpmutex.h"
using namespace std;

void cpmutex::lock()
{
	int c = 0;
	int z = 0;
	count++;
	c = locked.compare_exchange_strong(z, 1);
	while (!c) {
		syscall(SYS_futex, &locked, FUTEX_WAIT, 1, 0, 0, 0, -1);
		z = 0;
		c = locked.compare_exchange_strong(z, 1);
	}
}

void cpmutex::unlock()
{
	locked = 0;
	if (count-- != 1)
		syscall(SYS_futex, &locked, FUTEX_WAKE, 1, 0, 0, 0, -1);
}
