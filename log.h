#pragma once
#ifdef  _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif //  WIN32

//you can use this by 
//#include "log.h"
//it supports muti-thread
//needs C++11 or higher
//in unix-based system, use "-lpthread"

//a log example:
//Log::begin() << "Logsomething" << 1 << 3.14 << Log::end();
//or use macro
//LogFunc("Logsomething" << 1 << 3.14);


//default log file location : working dir, "./"

//default log file name : date + '-' + "test" + ".log"
//example: 20180524-text.log


#include <string>
#include <vector>
#include <functional>
#include <mutex>

using std::string;
using std::vector;
using std::function;

//log cache size
const int nMaxBufferSize = 30 * 1024;

//one log max size
const int nMaxPerLogSize = 1024;

//log file longest "path + name"size
const int nFileNameFormatSize = 50;

//one buffer has just one day log
//for example:it may no contain both 5-24 and 5-25 logs
class buffer
{
public:
	buffer() { 
		m_nEnd = 0;
		nToday = 0;
	}
	~buffer() {};
	char buf[nMaxBufferSize];
	int m_nEnd; //buffer ends position, buf[m_nEnd] = '\0'
	int nToday; //yday,1-365

	template<typename T>
	void writeformat(std::string format,T n)
	{
		m_nEnd += sprintf(buf + m_nEnd, format.c_str(), n);
	}

	void writeEnd();
	void append(char *outbuf, size_t size);
	void clear() { 
		m_nEnd = 0;
		nToday = 0;
	}
};

class Log
{
public:

	Log()
	{
		start();
	}

	~Log();

	class ender
	{

	};
	
	static Log& instance()
	{
		static Log ins;
		return ins;
	}

	static Log& begin();
	static ender end();
	void operator<< (ender n);

	//if you need to log some class, provide asString method
	//like this:
	//const char * Foo::asString();
	template<typename T>
	Log& operator<< (T m)
	{
		m_pBufferA->writeformat("%s,",m.asString());
	}
	Log& operator<< (int num);
	Log& operator<< (string str);
	Log& operator<< (const char *cstr);
	Log& operator<< (float f);
	Log& operator<< (double d);
	Log& operator<< (long long num);
	Log& operator<< (size_t size);

public:

	//if you need, use these function !!before you do logs;
	//use these function by : 
	//Log::instance().set_WriteFunc();

	//you can specify a writelog method, write to sql or sth
	void setm_WriteFunc(function<void(buffer *)> func) { m_WriteFunc = func; }

	//also you can name a log file
	//no too long name
	//chg name to "20180524-str.log"
	void setLogName(string str) {m_sFileName = str; }

	//and its file path
	//no too long path
	void setLogPath(string str) { m_sFilePath = str; } 

	//you can end WriteLog thread by this
	void detach() { m_bstop = true; }

private:
	static void WriteThreadEntrance(Log *self);
	void writeThread();
	void writeToLocal(buffer *);
	
private:
	void lock() { m_lock.lock(); }
	void unlock() { m_lock.unlock(); }


	void AddFormatTime(struct tm &t_now);
	void CheckCurrentBuffer(int nday);


private:

	void start();
	int nWriteFileDay;

	buffer *m_pBufferA; //Now Writing Buffer
	buffer *m_pBufferB; 
	buffer *m_pBufferC;
	buffer *m_pBufferD;
	vector<buffer *> m_vNeedWrite;

	vector<buffer *> m_vWrite;
	FILE *m_fp;
	string m_sFileName;
	string m_sFilePath;
	function<void(buffer *)> m_WriteFunc;

	std::mutex m_lock;
	std::thread w_thread;
	bool m_bstop;
};

#define LogFunc(msg) \
{\
	{\
		Log::begin() << msg << Log::end();\
	}\
}
