#ifndef __THREAD_WAIT__
#define __THREAD_WAIT__

#include <atomic>
#include <sstream>
class checkpoint;

class thread_wait {
public:
	thread_wait(checkpoint *_chkp, checkp_get _get, std::string _name):
	chkp(_chkp),
	get(_get),
	name(_name),
	futex(0)
	{}
	std::function<int ()> get;
//	std::function<int ()> set;
	int wait();
	int wake();
	std::string getName() { return name; }
	friend std::ostream& operator<<(std::ostream& os, const thread_wait& tw)
	{
		return os << "[" << tw.name << "]";
	}
private:
	checkpoint *chkp;
	std::atomic<int> futex;
	std::string name;
};

#endif
