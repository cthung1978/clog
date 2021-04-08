#include "clog/clog.h"
#include <cstring>
int main()
{
	// CLOG clog("logfile.txt");
	CLOG clog;
	int i = 3;
	clog.setTimeTagFormat("%H:%M:%S");
	// clog.setTimeTagFormat("%H-%M-%S");
	clog.setAutoflush(false);

	char s[10];
	memcpy(s, "1122\0", 5);
	clog.write(MSG, "test %s", s);
	clog.write(WAR, "test");
	clog.write(ERR, "test");

	clog.write(DB1, "test");
	clog.write(DB2, "test");
	clog.write(DB3, "test");

	// clog << string("string message 1")  << endl;
	// clog << string("string message 2")  << endl;
	// clog << string("string message 3")  << endl;
	clog << "helloc " << endl;

	// this_thread::sleep_for(std::chrono::seconds(10));
	return 0;
}
