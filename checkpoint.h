#ifndef __CHECKPOINT_H__
#define __CHECKPOINT_H__

#include <list>
#include <atomic>
#include <string>
#include <functional>
#include "cpmutex.h"

typedef std::function<int ()> checkp_get;
typedef std::function<int ()> checkp_set;
#include "thread_wait.h"

using namespace std;

class checkpoint {
friend thread_wait;
public:
	int pass(checkp_get get, checkp_set set, std::string name = "default");
	int try_pass(checkp_get get, checkp_set set);
	std::string print();
private:
	thread_wait *process_thread_waits();
	std::list<thread_wait *> tws;
	cpmutex m;
	void lock()		{	m.lock();	}
	void unlock()	{	m.unlock();	}
};
#endif
