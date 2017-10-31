#include <unistd.h>
#include <sys/syscall.h>
#include <bits/syscall.h>
#include <linux/futex.h>
#include "checkpoint.h"
#include "thread_wait.h"
using namespace std;

int thread_wait::wait()
{
	if (futex == 1)
		return 0;
	futex = 1;
	chkp->unlock();
	syscall(SYS_futex, &futex, FUTEX_WAIT, 1, 0, 0, 0, -1);
	return 0;
}

int thread_wait::wake()
{
	if (futex == 0)
		return 0;
	futex = 0;
	syscall(SYS_futex, &futex, FUTEX_WAKE, 0, 0, 0, 0, -1);
	return 0;
}

