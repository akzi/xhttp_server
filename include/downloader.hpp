#pragma once
#include <fstream>
namespace xhttp_server
{
	class downloader
	{
	public:
		downloader(request &_request)
			:request_(_request)
		{
		}
		bool send_file(const std::string &filepath)
		{
			filepath_ = filepath;
			if (!do_send_file())
				return false;
			return true;
		}
	private:
		void close()
		{
			request_.close();
		}
		bool do_send_file()
		{
			using get_extension = xutil::functional::get_extension;
			using get_filename = xutil::functional::get_filename;

			std::ios_base::openmode  mode = std::ios::binary | std::ios::in;
			file_.open(filepath_.c_str(), mode);
			if (!file_.good())
				return false;
			file_.seekg(0, std::ios::end);
			auto size = file_.tellg();
			http_builder_.append_entry("Content-Length", std::to_string(size).c_str());
			http_builder_.append_entry("Content-Type", http_builder_.get_content_type(get_extension()(filepath_)));
			http_builder_.append_entry("Content-Disposition", "attachment; filename=" + get_filename()(filepath_));
			std::string buffer;
			buffer.resize(102400);
			std::function<void()> resume_handle;
			xnet::connection &conn = request_.conn_;
			file_.seekg(0, std::ios::beg);
			conn.regist_send_callback([&](std::size_t len) {
				if (len == 0)
				{
					close();
					file_.close();
					resume_handle();
					return;
				}
				if (file_.eof())
				{
					file_.close();
					resume_handle();
					return;
				}
				file_.read((char*)buffer.data(), buffer.size());
				if(file_.gcount() > 0)
					conn.async_send(buffer.data(), (uint32_t)file_.gcount());
			});
			conn.async_send(std::move(http_builder_.build_resp()));
			xcoroutine::yield(resume_handle);
			return  true;
		}
		std::ifstream file_;
		std::string filepath_;
		request &request_;
		xhttper::http_builder http_builder_;
	};
}
