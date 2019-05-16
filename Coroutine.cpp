#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "Coroutine.h"
#include "Schedule.h"

namespace coroutine
{

Coroutine::Coroutine(const CoroutineFunc& func, int id):
		func_(func), id_(id), state_(kReady),stack_(NULL),
		capacity_(0),size_(0)
{
		
}

void Coroutine::start(Schedule* schedule)
{
	if(func_ != nullptr)
	{
		setState(kRunning);
		func_(schedule);//开始调度
	}
}

void Coroutine::saveStack(char* tpos)
{
	char dummy = 0;
	assert(tpos - &dummy <= static_cast<ptrdiff_t>(Schedule::kStackSize));
	//如果空间不够
	if(capacity_ < tpos - &dummy)
	{
		::free(stack_);
		capacity_ = tpos - &dummy;
		stack_ = reinterpret_cast<char*>(::malloc(static_cast<size_t>(capacity_)));
	}
	size_ = (tpos - &dummy);
	memcpy(stack_, &dummy, static_cast<size_t>(size_));
}

int Coroutine::id() const
{
	return id_;
}

Coroutine::State Coroutine::state() const
{
	return state_;
}

void Coroutine::setState(const State& sarg)
{
	state_ = sarg;
}

ucontext_t* Coroutine::getContextMutable()
{
	return &context_;
}

const ucontext_t& Coroutine::getContext()
{
	return context_;
}

char* Coroutine::stack() const
{
	return stack_;
}

ptrdiff_t Coroutine::size() const
{
	return size_;
}

ptrdiff_t Coroutine::capacity() const
{
	return capacity_;
}

Coroutine::~Coroutine()
{
	assert(state_ == kDead);
	if(stack_ != nullptr)
	{
		::free(stack_);
		stack_ = nullptr;
	}
}

}
