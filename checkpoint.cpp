#include "checkpoint.h"
using namespace std;

int checkpoint::pass(checkp_get get, checkp_set set, std::string name)
{
	lock();
	if (get() == 0) {
		thread_wait *tw = new thread_wait(this, get, name);
		tws.push_back(tw);
		tw->wait();
		delete tw;
		return pass(get, set, name);
	}
	int ret = set();
	thread_wait *tw = process_thread_waits();
	if (tw)
		tw->wake();
	unlock();
	return ret;
}

int checkpoint::try_pass(checkp_get get, checkp_set set)
{
	lock();
	if (get() == 0) {
		unlock();
		return -1;
	}
	int ret = set();
	thread_wait *tw = process_thread_waits();
	if (tw)
		tw->wake();
	unlock();
	return ret;
}

thread_wait *checkpoint::process_thread_waits()
{
	for(list<thread_wait *>::iterator it=tws.begin(); it != tws.end();) {
		thread_wait *tw = *it;
		if (tw->get() != 0) {
			it = tws.erase(it);
			return tw;
		} else
			it++;
	}
	return 0;
}

string checkpoint::print()
{
	stringstream os;
	lock();
	os << "Checkpoint state:\n";
	for (std::list<thread_wait *>::const_iterator it=tws.begin(); it!=tws.end(); it++) {
		os << *(*it);
	}
	unlock();
	return os.str();
}

