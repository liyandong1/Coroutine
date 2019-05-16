#include <iostream>
#include <functional>
#include <sys/time.h>
#include "Schedule.h"

using namespace std;
using namespace coroutine;
using namespace std::placeholders;

void func1(Schedule* schedule)
{
	while(1)
	{
		cout << "Id =" << schedule->getRunningCoroutineId() << endl;
		schedule->suspendCurrentCoroutine();
	}
}

void func2(Schedule* schedule)
{
	while(1)
	{
		cout << "Id =" << schedule->getRunningCoroutineId() << endl;
		schedule->suspendCurrentCoroutine();
	}
}

int main()
{
	Schedule schedule;
	int coutotine1 = schedule.createCoroutine(func1);
	int coutotine2 = schedule.createCoroutine(func2);
	const int kMicrosecondsPerSecond = 1000 * 1000;
	struct timeval tv_start;
	gettimeofday(&tv_start, NULL);
	int64_t start = static_cast<int64_t>(tv_start.tv_sec) * kMicrosecondsPerSecond + tv_start.tv_usec;
	
	for(int i = 0; i < 1000000; i ++)
	{
		schedule.runCoroutineById(coutotine1);
		schedule.runCoroutineById(coutotine2);
	}
	
	
	struct timeval tv_end;
	gettimeofday(&tv_end, NULL);
	int64_t end = static_cast<int64_t>(tv_end.tv_sec) * kMicrosecondsPerSecond + tv_end.tv_usec;
	cout << "Total time of switching 40M times: " << \
			static_cast<double>(end - start)/kMicrosecondsPerSecond << "s" << endl;
	return 0;
}
