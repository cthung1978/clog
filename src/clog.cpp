#include <cstdarg>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include "clog/clog.h"

CLOG::CLOG():logStream(NULL)
{
	string strempty;
	strempty.clear();
	init(strempty);
}

CLOG::CLOG(string _filename):logStream(NULL)
{
	init(_filename);
}

void CLOG::init(string _filename)
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
	to_unlock(writeLock);

	filename = _filename;
	flag_newline = true;
	setFilename(filename);
	stopThread = 0;
	logThread = new std::thread( &CLOG::logThreadFunc , this);
}

CLOG::~CLOG()
{
	struct clogMessage *clog_record;

	// Tell the thread to stop
	release();

	// Sleep for a while and wait the thread to clean the msg queue
	this_thread::sleep_for(std::chrono::milliseconds(200));
	logThread->join();

	while(msgQueue.pop(clog_record))
	{
		delete clog_record;
	}

	while(msgPool.pop(clog_record))
	{
		delete clog_record;
	}

	delete [] logLevelTags;

	if (logFileStream)
		logFileStream.close();
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
	char msg[CLOG_MAX_MSG_SIZE];
	if (_filename.empty())
	{
		fileStreamBuf = cout.rdbuf();
		logStream.rdbuf(fileStreamBuf);
		return;
	}

	to_lock(writeLock);
	if (logFileStream.is_open())
	{
		sprintf(msg, "%s is already opened, closing...", filename.c_str());
		write(WAR, msg);
		logFileStream.close();
	}

	filename = _filename;
	logFileStream.open(filename, std::ofstream::out | std::ofstream::app);
	if(logFileStream.is_open())
	{
		fileStreamBuf = logFileStream.rdbuf();
		logStream.rdbuf(fileStreamBuf);
	} else
	{
		fileStreamBuf = cout.rdbuf();
		logStream.rdbuf(fileStreamBuf);
	}
	to_unlock(writeLock);
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

void CLOG::write(LOGLEVEL expectLogLevel, const char *msgFmt, ...)
{
	va_list args;
	char msg[CLOG_MAX_MSG_SIZE];
	string timeTag = getTimeTag();
	string message;
	struct clogMessage *msg_record;

	if (logLevel < expectLogLevel || stopThread )
	{
		return;
	}

	va_start(args, msgFmt);
	vsnprintf(msg, CLOG_MAX_MSG_SIZE, msgFmt, args);
	va_end(args);
	cout << "msg " << msg << endl;
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

}


void CLOG::logThreadFunc()
{
	bool runFlag;
	struct clogMessage *msg_record;
	struct timeval *timer, nowtime;

	timer = NULL;
	runFlag = true;
	while(runFlag)
	{
		if(msgQueue.pop(msg_record))
		{
			to_lock(writeLock);
			logStream << msg_record->msg;
			if (flagAutoFlush) logStream.flush();
			msg_record->inUse = false;
			msg_record->msg[0] = '\0';
			msgPool.push(msg_record);
			to_unlock(writeLock);
			continue;
		}

		if (stopThread)
		{
			*this << endl << " " << endl;
			runFlag = false;
		}

		// not so buzy loop
		this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	if (timer)
	{
		free(timer);
	}
}

CLOG& CLOG::operator<<( endl_type endl )
{
	string str;
	str.clear();
	ssBuffer >> str;
	write(MSG, "%s", str.c_str());
	flag_newline = true;
	return *this;
}
