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
		bool parser_request(const std::string &path)
		{
			path_ = path;
			content_length_ = request_.content_length();
			if (!content_length_)
				return false;
			if (!check_content_type())
				return  false;
			do_parser();
			return true;
		}

	private:
		void data_callback(char *data, std::size_t len)
		{
			std::string buffer_;
		}
		bool check_content_type()
		{
			std::string content_type = request_.get_header("Content-Type");
			if (content_type.empty())
				return false;
			if (content_type.find("multipart/form-data") == std::string::npos)
				return false;
			auto pos = content_type.find("boundary=");
			if (pos == std::string::npos)
				return false;
			pos += strlen("boundary=");
			boundary_ = content_type.substr(pos, content_type.size() - pos);
			return true;
		}
		void do_parser()
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
				data_callback(data, len);
			});
			conn.async_recv_some();
			auto buffer = request_.parser_.get_string();
			data_callback((char*)buffer.data(), buffer.length());
			xcoroutine::yield(resume_handle);
		}
		std::size_t content_length_ = 0;
		bool result_ = false;
		std::string path_;
		std::string boundary_;
		std::map<std::string, std::string> fields_;
		std::map<std::string, std::string> files_;
		request &request_;
	};
}