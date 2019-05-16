#ifndef _COROUTINE_COROUTINE_H_
#define _COROUTINE_COROUTINE_H_

#include <memory>
#include <functional>
#include <stddef.h>
#include <ucontext.h>

namespace coroutine
{

class Schedule;
class Coroutine
{
public:
	//协程状态
	enum State
	{
		kInvalid,
		kDead,
		kReady,
		kRunning,
		kSuspend
	};
	
	typedef std::function<void(Schedule*)> CoroutineFunc; //协程调度函数
	
	Coroutine(const CoroutineFunc& func, int id); //当前协程要执行的函数执行流
	
	void start(Schedule* schedule);
	void saveStack(char* tpos);
	int id() const;
	State state() const;
	void setState(const State& sarg);
	
	ucontext_t* getContextMutable();
	const ucontext_t& getContext();
	char* stack() const;
	ptrdiff_t size() const;
	ptrdiff_t capacity() const;
	
	~Coroutine();
private:
	CoroutineFunc func_;
	int id_;
	State state_;
	ucontext_t context_;//当前执行流的信息
	char* stack_;
	ptrdiff_t capacity_; //ptr diatance
	ptrdiff_t size_;
};

typedef std::shared_ptr<Coroutine> CoroutinePtr; 


}

#endif
