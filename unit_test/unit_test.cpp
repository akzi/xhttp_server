#include "../include/xhttp_server.hpp"
#include "../../xtest/include/xtest.hpp"

xtest_run;

XTEST_SUITE(xhttp_server)
{
	using namespace xhttp_server;

	void hello_world(request &req, response &rsp)
	{
		rsp.set_data("hello world");
		rsp.done();
	}
	XUNIT_TEST(regist)
	{
		xserver server;
		server.bind("0.0.0.0", 9001);
		server.regist(hello_world);
		server.start();
		getchar();
	}
}