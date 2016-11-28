#include "../include/xhttp_server.hpp"
#include "../../xtest/include/xtest.hpp"

xtest_run;

XTEST_SUITE(xhttp_server)
{
	using namespace xhttp_server;

	void hello_world(request &req, response &rsp)
	{
		auto session =  req.get_session();
		xassert(session.set("hello", "world"));
		std::string result;
		xassert(session.get("hello",result));
		xassert(result == "world");
		rsp.set_data("hello world");
		rsp.done();
	}
	XUNIT_TEST(regist)
	{
		xserver server;
		server.bind("0.0.0.0", 9001);
		server.set_redis_addr("192.168.0.2",6379);
		server.regist(hello_world);
		server.start();
		getchar();
	}
}