#pragma once
namespace xhttp_server
{
	class file_uploader
	{
	public:
		file_uploader(request &_request)
			:request_(_request)
		{
		}
		bool save(const std::string &path)
		{
			path_ = path;
			do_save();
			return true;
		}
	private:
		void data_callback(char *data, std::size_t len)
		{

		}
		bool check_content_type()
		{

		}
		void do_save()
		{
			std::function<void()> resume_handle;
			xnet::connection &conn = request_.conn_;
			conn.regist_recv_callback([&](char *data, std::size_t len) {
				if (len == 0)
				{
					result_ = false;
					conn.close();
					resume_handle();
				}
			});
			conn.async_recv_some();
			xcoroutine::yield(resume_handle);
		}
		bool result_;
		std::string path_;
		request &request_;
	};
}