#ifdef  _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif //  WIN32

#include "Log.h"
#include <thread>
#include <string.h>
#include <time.h>

using namespace std;

void buffer::append(char *outbuf, size_t size)
{
	m_nEnd += sprintf(buf + m_nEnd, "%*s", size, outbuf);
}

void buffer::writeEnd()
{
	buf[m_nEnd - 1] = '\n';
	buf[m_nEnd] = '\0';
}

void Log::start()
{
	m_pBufferA = new buffer();
	m_pBufferB = new buffer();
	m_pBufferC = new buffer();
	m_pBufferD = new buffer();
	m_WriteFunc = bind(&Log::writeToLocal,this, placeholders::_1);
	m_sFileName = "test";
#ifdef _WIN32
	m_sFilePath = ".\\";
#else
	m_sFilePath = "./";
#endif
	m_bstop = false;
	w_thread = thread(Log::WriteThreadEntrance, this);
}

Log::~Log()
{
	w_thread.join();
}

void Log::WriteThreadEntrance(Log *Log)
{
	Log->writeThread();
}

void Log::writeThread()
{
	for (;;)
	{

		if (m_vNeedWrite.size() <= 0 && m_pBufferA->m_nEnd == 0)
		{
			//rest
			if (m_bstop)
				return;
			std::this_thread::sleep_for(0.5s);
//			Log::begin() << "write sleeping " << Log::end();
			continue;
		}
		m_vWrite.clear();
//临界+  ---------
		lock();
		if(m_pBufferA->m_nEnd != 0)
		{ 
			m_vNeedWrite.push_back(m_pBufferA);
		}

		for (int i = 0; i < m_vNeedWrite.size(); i++)
		{
			m_vWrite.push_back(m_vNeedWrite.at(i));
		}

		m_pBufferA = m_pBufferC;
		if(m_vNeedWrite.size() > 1)	m_pBufferB = m_pBufferD; //可能没写

		m_vNeedWrite.clear();
		unlock();
//临界-  ----------
		for (int i = 0; i < m_vWrite.size(); i++)
		{
			buffer *pBuffer = m_vWrite.at(i);
			m_WriteFunc(pBuffer);
		}

		m_vWrite[0]->clear();
		m_pBufferC = m_vWrite[0];

		if (m_vNeedWrite.size() > 1)
		{
			m_vWrite[1]->clear();
			m_pBufferD = m_vWrite[1];
		}

//		if (m_vWrite.size() > 2)
//			Log::begin() << "Log more buffer allocate " << int(m_vWrite.size()) << Log::end();
		for (int i = 2; i < m_vWrite.size(); i++)
		{
			buffer *pBuffer = m_vWrite.at(i);
			delete pBuffer;
		}
	}
}

void Log::writeToLocal(buffer * buf_)
{
	if (m_fp == NULL || buf_->nToday != nWriteFileDay)
	{
		static char timeFormat[10];
		static char fileFormat[nFileNameFormatSize];
		size_t nsize = 0;
		nsize = sprintf(fileFormat, "%s", m_sFilePath.c_str());
		time_t now = time(NULL);
		struct tm t_now = *localtime(&now);
		nWriteFileDay = t_now.tm_yday;
		size_t ntimesize = strftime(timeFormat, 20, "%Y%m%d", &t_now);
		nsize += sprintf(fileFormat + nsize, "%*s", ntimesize,timeFormat);
		nsize += sprintf(fileFormat + nsize, "-%s.log",m_sFileName.c_str());
		fileFormat[nsize] = '\0';

		if (m_fp)
			fclose(m_fp);
		m_fp = fopen(fileFormat, "a+");
	}
	fprintf(m_fp, "%s", buf_->buf);
}

Log& Log::begin()
{
	Log &ins = Log::instance();
	time_t now = time(NULL);
	struct tm t_now = *localtime(&now);
	ins.lock();
//---- 临界+
	ins.CheckCurrentBuffer(t_now.tm_yday);
	ins.AddFormatTime(t_now);
	return ins;
}

Log::ender Log::end()
{
	static Log::ender end_;
	return end_;
}

void Log::operator<<(Log::ender a)
{
	m_pBufferA->writeEnd();
	unlock();
// --- 临界-
}

Log& Log::operator <<(int num)
{
	m_pBufferA->writeformat<int>("%d,",num);
	return *this;
}

Log& Log::operator<<(string str)
{
	m_pBufferA->writeformat<const char*>("%s,", str.c_str());
	return *this;
}

Log& Log::operator<<(const char* str)
{
	m_pBufferA->writeformat<const char*>("%s,", str);
	return *this;
}

Log& Log::operator<<(size_t size)
{
	m_pBufferA->writeformat<int>("%d,", size);
	return *this;
}

Log& Log::operator<<(float f)
{
	m_pBufferA->writeformat<float>("%f,", f);
	return *this;
}

Log& Log::operator<<(double d)
{
	m_pBufferA->writeformat<double>("%lf,", d);
	return *this;
}

Log& Log::operator<<(long long big)
{
	m_pBufferA->writeformat<long long>("%lld", big);
	return *this;
}

void Log::CheckCurrentBuffer(int nday)
{
	if (m_pBufferA->nToday == 0)
		m_pBufferA->nToday = nday;

	if (nMaxBufferSize - m_pBufferA->m_nEnd - 2 > nMaxPerLogSize && nday == m_pBufferA->nToday)
		return;

	m_vNeedWrite.push_back(m_pBufferA);
	if (m_pBufferB->m_nEnd == 0)
	{//交换
		buffer *tempBuffer;
		tempBuffer = m_pBufferA;
		m_pBufferA = m_pBufferB;
		m_pBufferB = m_pBufferA;
	}
	else
	{//新建缓冲
		m_pBufferA = new buffer();
	}
}

void Log::AddFormatTime(struct tm &t_now)
{
	static char timeFormat[50];
	size_t nsize = strftime(timeFormat, 50, "%Y-%m-%d %I:%M:%S ", &t_now);
	m_pBufferA->append(timeFormat,nsize);
}