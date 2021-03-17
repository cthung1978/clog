#ifndef __CLOG_H
#define __CLOG_H

#include <cstdio>
#include <string>
#include <thread>
#include <boost/lockfree/queue.hpp>

using namespace std;

#define CLOG_MAX_MSG_SIZE 1024 
#define CLOG_MSG_POOL_SIZE 4096 

#ifdef __cplusplus
extern "C" {
#endif 

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
		void write(LOGLEVEL, const char *msgFmt, ...);
		void setTimeTagFormat(const char *);
		void release();

	private:
		int flagAutoFlush;
		FILE *logfile;
		LOGLEVEL logLevel;
		char timeTagFormat[64];
		string *logLevelTags;

		struct clogMessage *msgPool;
		boost::lockfree::queue<char *> msgQueue;
		
		string getTimeTag();

		int stopThread;
		std::thread logThread;
		void logThreadFunc();

		// time and tag format
		
};


#ifdef __cplusplus
}
#endif

#endif
