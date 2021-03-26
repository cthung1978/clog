#include "clog/clog.h"

int main()
{
	// CLOG clog("logfile.txt");
	CLOG clog;

	clog.setTimeTagFormat("%H:%M:%S");
	// clog.setTimeTagFormat("%H-%M-%S");
	clog.setAutoflush(true);

	clog.write(MSG, "test");
	clog.write(WAR, "test");
	clog.write(ERR, "test");

	// clog.setFilename("log.txt");

	clog.write(DB1, "test");
	clog.write(DB2, "test");
	clog.write(DB3, "test");

	// clog << string("string message 1")  << endl;
	// clog << string("string message 2")  << endl;
	// clog << string("string message 3")  << endl;

	this_thread::sleep_for(std::chrono::seconds(10));
	clog.release();
	return 0;
}
