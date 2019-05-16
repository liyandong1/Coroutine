#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "Schedule.h"

#include <iostream>
using namespace std;

namespace coroutine
{

void coroutineFunc(Schedule* schedule)
{
	int id = schedule->getRunningCoroutineId();
	
	const CoroutinePtr& coroutine = schedule->getCoroutineById(id);
	if(coroutine != nullptr)
	{
		coroutine->start(schedule);
	}
	
	//当前协程已经执行过了
	coroutine->setState(Coroutine::kDead);
	schedule->deleteCoroutineById(id);
	schedule->setRunningCoroutineId(-1);
}

const int Schedule::kStackSize;

//控制协程池数量
Schedule::Schedule(int size):
	kCapacity_(size), runningId_(-1),flags_(size, 0)
{
	
}

int Schedule::createCoroutine(const CoroutineFunc& func)
{
	if(coroutines_.size() < static_cast<size_t>(kCapacity_))
	{
		for(int i = 0; i < kCapacity_; i ++)
		{
			int id = static_cast<int>(coroutines_.size() + i)% kCapacity_;
			if(flags_[id] == 0)
			{
				CoroutinePtr newCoroutine(new Coroutine(func, id));
				flags_[id] = 1;
				coroutines_[id] = newCoroutine;
				return id;
			}
		}
		assert(false); //永远不应该执行到这里
		return -1;
	}
	return -1;
}

//通过id调用协程
void Schedule::runCoroutineById(int id)
{
	assert(runningId_ == -1);
	assert((0 <= id) && (id < kCapacity_));
	auto it = coroutines_.find(id);
	if(it == coroutines_.end())
		return;
	const CoroutinePtr& coroutine = it->second;
	if(coroutine != nullptr)
	{
		assert(id == coroutine->id());
		Coroutine::State state = coroutine->state();
		if(state == Coroutine::kReady) //就绪态
		{
			//得到当前协程的上下文信息,保存在协程结构体中
			getcontext(coroutine->getContextMutable());
			coroutine->getContextMutable()->uc_stack.ss_sp = stack_;//统一使用。。。。
			coroutine->getContextMutable()->uc_stack.ss_size = kStackSize;
			coroutine->getContextMutable()->uc_stack.ss_flags = 0;
			//指定下一个被调度的协程,设置为主协程
			coroutine->getContextMutable()->uc_link = &mainContext_;
			
			runningId_ = id;
			coroutine->setState(Coroutine::kRunning);
			
			//在协程上下文中修改（添加）需要执行的函数信息
			makecontext(coroutine->getContextMutable(), \
					reinterpret_cast<void(*)()>(coroutineFunc), 1, this);
			
			//保存主协程上下文，并且激活目标上下文，通过link
			//目标上下文执行完又会切换到主协程上下文
			swapcontext(&mainContext_, coroutine->getContextMutable());	
		}
		else if(state == Coroutine::kSuspend)
		{	
			//栈从高到低
			memcpy(stack_ + kStackSize - coroutine->size(), coroutine->stack(), coroutine->size());
			runningId_ = id;
			coroutine->setState(Coroutine::kRunning);
			swapcontext(&mainContext_, coroutine->getContextMutable());	
		}
	}
}

void Schedule::suspendCurrentCoroutine()
{
	assert((0 <= runningId_) && (runningId_ < kCapacity_));
	auto it = coroutines_.find(runningId_);
	assert(it != coroutines_.end());
	const CoroutinePtr& coroutine = it->second;
	assert(coroutine != nullptr);
	
	assert(runningId_ == coroutine->id());
	coroutine->saveStack(stack_ + kStackSize); //栈从高到低
	coroutine->setState(Coroutine::kSuspend);
	runningId_ = -1;
	//切换到主协程
	swapcontext(coroutine->getContextMutable(), &mainContext_);	
}

Coroutine::State Schedule::getCoroutineStateById(int id) const
{
	auto it = coroutines_.find(id);
	if(it == coroutines_.end())
		return Coroutine::kInvalid;
	const CoroutinePtr& coroution = it->second;
	if(coroution != nullptr)
	{
		assert(id == coroution->id());
		return coroution->state();
	}
	return Coroutine::kInvalid;
}

int Schedule::getRunningCoroutineId() const
{
	return runningId_;
}

CoroutinePtr Schedule::getCoroutineById(int id) const
{
	auto it = coroutines_.find(id);
	if(it != coroutines_.end())
		return it->second;
	return nullptr;
}

void Schedule::deleteCoroutineById(int id)
{
	auto it = coroutines_.find(id);
	if(it == coroutines_.end())
		return;
	coroutines_.erase(it);
	flags_[id] = 0;
}

void Schedule::setRunningCoroutineId(int id)
{
	runningId_ = id;
}

Schedule::~Schedule()
{
	
	assert(runningId_ == -1);
}


}
