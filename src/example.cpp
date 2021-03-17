#include "clog/clog.h"

int main()
{
	CLOG clog("log.txt");

	clog.setAutoflush(true);

	clog.write(MSG, "test");
	clog.write(WAR, "test");
	clog.write(ERR, "test");
	clog.write(DB1, "test");
	clog.write(DB2, "test");
	clog.write(DB3, "test");

	// clog.release();
	this_thread::sleep_for(std::chrono::seconds(10));
	return 0;
}
