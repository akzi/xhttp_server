#include "../include/xhttp_server.hpp"
#include "../../xtest/include/xtest.hpp"

xtest_run;

XTEST_SUITE(xhttp_server)
{
	using namespace xhttp_server;

	void redis_session_test(request &req, response &rsp)
	{
		auto session =  req.get_session();
		xassert(session.set("hello", "world"));
		std::string result;
		xassert(session.get("hello",result));
		xassert(result == "world");
		rsp.set_data("hello world");
		rsp.done();
	}

	void upload_file_test(request &req, response &rsp)
	{
		uploader _uploader(req);
		_uploader.parser_request("");
		std::cout << std::endl;
		for(auto &itr: _uploader.get_field())
			std::cout << itr.first << ": " << itr.second << std::endl;
		for (auto &itr : _uploader.get_files())
			std::cout << itr.first << ": " << itr.second << std::endl;
		rsp.set_data("ok");
		rsp.done();
	}

	void download_file_test(request &req, response &rsp)
	{
		downloader download_file(req);
		download_file.send_file("D:/Ñ¸À×ÏÂÔØ/msys2-x86_64-20160205.exe");
	}
	XUNIT_TEST(regist)
	{
		xserver server;
		server.bind("0.0.0.0", 9001);
		//server.set_redis_addr("192.168.0.2",6379);
		server.regist(download_file_test);
		server.start(1);
		getchar();
	}
}
 