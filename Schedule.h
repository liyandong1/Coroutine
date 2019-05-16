//This is a public header file,you should include this

#ifndef _COROUTINE_SCHEDULE_H_
#define _COROUTINE_SCHEDULE_H_

#include <map>
#include <vector>
#include <stdint.h>
#include <ucontext.h>
#include "Coroutine.h"

namespace coroutine
{

//协程池，调度器
class Schedule
{
public:
	static const int kStackSize = 1024 * 1024;
	typedef std::function<void(Schedule*)> CoroutineFunc;
	
	Schedule(int size = 16);
	//if error, return -1
	int createCoroutine(const CoroutineFunc& func);
	void runCoroutineById(int id);
	void suspendCurrentCoroutine();//暂停当前协程
	Coroutine::State getCoroutineStateById(int id) const;
	
	//得到正在执行的协程id
	int getRunningCoroutineId() const;
	~Schedule();
private:
	friend void coroutineFunc(Schedule* schedule);
	CoroutinePtr getCoroutineById(int id) const;
	void deleteCoroutineById(int id);
	void setRunningCoroutineId(int id); //正在执行的协程id
	
	typedef std::map<int, CoroutinePtr> CoroutineMap;
	
	const int kCapacity_;//协程最大数量
	int runningId_;
	ucontext_t mainContext_;  //主协程的上下文信息
	char stack_[kStackSize];  //maybe main coroutine id
	CoroutineMap coroutines_;
	std::vector<int> flags_;  //在map对照使用
};

}


#endif
