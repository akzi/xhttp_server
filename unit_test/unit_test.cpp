#include "../include/xhttp_server.hpp"
#include "../../../xtest/include/xtest.hpp"

xtest_run;

XTEST_SUITE(xhttp_server)
{
	using namespace xhttp_server;

	void handle_req(request &req, response &rsp)
	{
		req.get_body([&req, &rsp](std::string &&body) {

		});
	}
	XUNIT_TEST(regist)
	{
		xserver server;
		server.regist(handle_req);
	}
}