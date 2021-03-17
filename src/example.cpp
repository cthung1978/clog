#include "clog/clog.h"

int main()
{
	CLOG clog;

	clog.write(MSG, "test");
	clog.write(WAR, "test");
	clog.write(ERR, "test");
	clog.write(DB1, "test");
	clog.write(DB2, "test");
	clog.write(DB3, "test");

	clog.release();
	return 0;
}
