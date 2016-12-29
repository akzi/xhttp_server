# xhttp_server
http server,simple and easy

```cpp
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

	int main
	{
		xserver server;
		server.bind("0.0.0.0", 9001);
		server.set_redis_addr("192.168.0.2",6379);
		server.regist(redis_session_test);
		server.start(1);
		getchar();
		return 0;
	}
```
