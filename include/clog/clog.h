#ifndef __CLOG_H
#define __CLOG_H

#include <cstdio>
#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/lockfree/queue.hpp>

using namespace std;

#define CLOG_MAX_MSG_SIZE 1024
#define CLOG_MSG_POOL_SIZE 4096

#define to_lock(latch) while (__sync_lock_test_and_set(&(latch), 1)) while (latch)
#define to_unlock(latch) __sync_lock_release(&(latch))

struct clogMessage
{
	bool inUse;
	char msg[CLOG_MAX_MSG_SIZE];
};

typedef enum
{
	MSG, WAR, ERR, DB1, DB2, DB3, DB4, LOGLEVELCOUNT
} LOGLEVEL;

class CLOG
{
	public:
		CLOG();
		CLOG(string filename);
		~CLOG();

		void init(string _filename);
		void write(LOGLEVEL, const char *msgFmt, ...);
		void setTimeTagFormat(const char *);
		void setFilename(string);
		void setAutoflush(bool);
		void release();

		using endl_type = std::ostream&(std::ostream&);
		CLOG& operator<<( endl_type endl);

		template<typename T> CLOG& operator<< (const T& data)
		{
			string timeTag;
			string str;
			ssBuffer << data;
			return *this;
		};
	private:
		bool flagAutoFlush;
		ofstream logFileStream;
		std::streambuf *fileStreamBuf;
		stringstream ssBuffer;
		ostream logStream;

		string filename;
		LOGLEVEL logLevel;
		char timeTagFormat[64];
		string *logLevelTags;
		boost::lockfree::queue<struct clogMessage *, boost::lockfree::capacity<CLOG_MSG_POOL_SIZE> > msgPool;
		boost::lockfree::queue<struct clogMessage *, boost::lockfree::capacity<CLOG_MSG_POOL_SIZE> > msgQueue;

		string getTimeTag();

		int stopThread;
		std::thread *logThread;
		void logThreadFunc();

		volatile int writeLock;
		// time and tag format

		bool flag_newline;

};

#endif
