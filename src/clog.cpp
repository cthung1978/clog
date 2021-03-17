#include <cstdarg>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include "clog/clog.h"

CLOG::CLOG()
{
	CLOG("");
}

CLOG::CLOG(string _filename)
{
	int i, j;
	struct clogMessage *clog_record;

	setTimeTagFormat("%H:%M:%S");
	writeLock = 0;
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
			logLevelTags[i] = logLevelTags[i] + "  ";
	}

	for (i = 0; i < CLOG_MSG_POOL_SIZE; i++)
	{
		clog_record = new struct clogMessage;
		clog_record->inUse = false;
		clog_record->msg[0] = '\0';
		msgPool.push(clog_record);
	}

	flagAutoFlush = false;

	logfile = NULL;
	filename = _filename;
	setFilename(filename);

	stopThread = 0;
	logThread = std::thread( &CLOG::logThreadFunc , this);
}

CLOG::~CLOG()
{
	struct clogMessage *clog_record;

	while (__sync_lock_test_and_set(&(writeLock), 1)) while (writeLock) {};

	while(msgQueue.pop(clog_record))
	{
		delete clog_record;
	}

	while(msgPool.pop(clog_record))
	{
		delete clog_record;
	}

	if (logfile)
		fclose(logfile);

	stopThread = 1;
	logThread.join();
	delete [] logLevelTags;

	__sync_lock_release(&(writeLock));
}

void CLOG::setTimeTagFormat(const char *fmt)
{
	strncpy(timeTagFormat, fmt, 64);
}

void CLOG::setAutoflush(bool _flag)
{
	flagAutoFlush = _flag;
}

void CLOG::setFilename(string _filename)
{
	if (_filename.empty())
	{
		// do nothing
		return;
	}

	if (logfile != NULL)
	{
		fclose(logfile);
		logfile = NULL;
	}

	filename = _filename;
	logfile = fopen(filename.c_str(), "at");

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
	snprintf(buf2, 128, "%s.%06d ", buf1, microsec);
	timeTag = string(buf2);
	return timeTag;
}

void CLOG::write (LOGLEVEL expectLogLevel, const char *msgFmt, ...)
{
	va_list args;
	char msg[CLOG_MAX_MSG_SIZE];
	string timeTag = getTimeTag();
	string message;
	struct clogMessage *msg_record;

	if (logLevel < expectLogLevel)
		return;

	while (__sync_lock_test_and_set(&(writeLock), 1)) while (writeLock) {};

	va_start(args, msgFmt);
	snprintf(msg, CLOG_MAX_MSG_SIZE, msgFmt, args);
	va_end(args);

	message = timeTag + logLevelTags[expectLogLevel] + msg + '\n';

	msg_record = NULL;
	if (!msgPool.pop(msg_record))
	{
		msg_record = new struct clogMessage;
	}
	msg_record->inUse = true;
	msg_record->msg[0] = '\0';
	strncpy(msg_record->msg, message.c_str(), CLOG_MAX_MSG_SIZE);
	msgQueue.push(msg_record);

	__sync_lock_release(&(writeLock));
}

void CLOG::logThreadFunc()
{
	struct clogMessage *msg_record;

	while(!stopThread)
	{
		if(msgQueue.pop(msg_record))
		{
			if (logfile)
			{
				fprintf(logfile, "%s", msg_record->msg);
				if (flagAutoFlush)
				{
					fflush(logfile);
				}
			} else
			{
				cout << msg_record->msg ;
			}
			msg_record->inUse = false;
			msg_record->msg[0] = '\0';
			msgPool.push(msg_record);
			continue;
		}
		this_thread::sleep_for(std::chrono::seconds(1));
	}
}
