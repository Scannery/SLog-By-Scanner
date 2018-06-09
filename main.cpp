#ifdef  _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif //  WIN32

#include "log.h"
#include <thread>
#include <time.h>

using namespace std;

void writeFunc()
{
	for (int i = 0; i < 20000; i++)
	{
		Log::begin() << "gogogogogoog" << 312224422747 << 324 << Log::end();
	}
	return;
}

int main()
{
	Log::instance().setLogName("fucking");
	thread t1(writeFunc);
	thread t2(writeFunc);
	thread t3(writeFunc);
	t1.join();
	t2.join();
	t3.join();
	Log::instance().detach();
	return 0;
}