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
		_uploader.parse_request("");
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
		download_file.send_file("index.html");
	}
	void index(request &req, response &rsp)
	{
		rsp.send_file("index.html");
		rsp.done();
	}
	void filelist_test(request &req, response &resp)
	{
		auto path = req.req_path();
		if (path.back() == '/' || path.back() == '\\')
		{
			filelist fl(resp);
			return fl.resp_file_list(path);
		}
		downloader download_file(req);
		if (!download_file.send_file(xutil::vfs::getcwd()() +  path))
		{
			resp.set_status(404);
			resp.done();
		}
	}
	std::string async_get_str(int value)
	{
		return std::to_string(value);
	}

	void async_test(request &req, response &resp)
	{
		auto value = async(async_get_str, 1);

		resp.set_data(value);
		resp.done();
	}

	void hello (request &req, response &resp)
	{
		resp.set_status(200);
		resp.add_entry("Date", "Fri, 28 Oct 2016 12:43:43 GMT");
		resp.add_entry("Connection", "keep-alive");
		resp.set_data("hello");
		resp.done();
	}

	XUNIT_TEST(regist)
	{
		xserver server;
		server.bind("0.0.0.0", 9001);
		//server.set_redis_addr("192.168.0.2",6379);
		server.regist(hello);
		server.regist_run_before([&] {
			xhttp_server::init_async(server.get_proactor_pool().get_current_msgbox(), 1);
		});
		server.start();
		getchar();
	}
}
 
