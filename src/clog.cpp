#include <cstdarg>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include "clog/clog.h"

CLOG::CLOG()
{
	int i, j;
	

	setTimeTagFormat("%H:%M:%S");

	logLevel = DB4;
	logLevelTags = new string[LOGLEVELCOUNT];
	// i = 0, 1, 2 preserve for MSG WAR ERR
	logLevelTags[MSG] = "[MSG] ";
	logLevelTags[WAR] = "[WAR] ";
	logLevelTags[ERR] = "[ERR] ";
	for (i = 3; i < LOGLEVELCOUNT; i++)
	{
		logLevelTags[i] = "[DB" + to_string(i-2) + "] ";
		for (j = 1; j <= i - 2; j++)
			logLevelTags[i] = logLevelTags[i] + "\t";
	}

	msgPool = new struct clogMessage [CLOG_MSG_POOL_SIZE];
	for (i = 0; i < CLOG_MSG_POOL_SIZE; i++)
	{
		msgPool[i].inUse = false;
		msgPool[i].msg[0] = '\0';
	}

	stopThread = 0;
	logThread = std::thread( &CLOG::logThreadFunc , this);

}

CLOG::~CLOG()
{

	stopThread = 1;
	logThread.join();
	delete [] logLevelTags;
}

void CLOG::setTimeTagFormat(const char *fmt)
{
	strncpy(timeTagFormat, fmt, 64);
}

void CLOG::release()
{
	stopThread = 1;
}

string CLOG::getTimeTag()
{
	int rc;
	time_t rawtime;
	struct timeval tv;
	struct tm *timeinfo;
	int microsec;
	char buf1[128];
	char buf2[128];

	string timeTag = "";

	// To get time
	gettimeofday(&tv, NULL);
	microsec = tv.tv_usec;
	rawtime = tv.tv_sec;
	timeinfo = localtime(&rawtime);
	strftime(buf1, 128, timeTagFormat, timeinfo);
	snprintf(buf2, 128, "%s.%6ld ", buf1, microsec);
	timeTag = string(buf2);
	return timeTag;
}

void CLOG::write (LOGLEVEL expectLogLevel, const char *msgFmt, ...)
{
	va_list args;
	char msg[CLOG_MAX_MSG_SIZE];
	string timeTag = getTimeTag();
	string message;

	if (logLevel < expectLogLevel)
		return;

	va_start(args, msgFmt);
	snprintf(msg, CLOG_MAX_MSG_SIZE, msgFmt, args);
	va_end(args);

	message = timeTag + logLevelTags[expectLogLevel] + msg + '\n';
	//msgPool.push(message);

}

void CLOG::logThreadFunc()
{
	string message;
	while(!stopThread)
	{ 
		//if(msgPool.pop(message))
		{
			cout << message ;
			continue;
		}
		this_thread::sleep_for(std::chrono::seconds(1));
	}
}
