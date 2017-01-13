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
			using get_rfc1123 = xutil::functional::get_rfc1123;
			auto range = request_.get_range();
			int64_t begin = 0;
			int64_t end = 0;
			std::ios_base::openmode  mode;
			
			mode = std::ios::binary | std::ios::in;
			file_.open(filepath_.c_str(), mode);
			if (!file_.good())
				return false;

			file_.seekg(0, std::ios::end);
			auto size = file_.tellg();
			end = size;

			bool has_range = false;
			if (range.first != UINT64_MAX)
			{
				begin = range.first;
				has_range = true;
			}
			if (range.second != UINT64_MAX)
			{
				has_range = true;
				if (range.first == UINT64_MAX)
				{
					end = size;
					begin = (int64_t)size - range.second;
				}
				else
				{
					end = range.second;
				}
			}

			http_builder_.append_entry("Date", get_rfc1123()());
			http_builder_.append_entry("Connection", "keep-alive");
			http_builder_.append_entry("Content-Type", http_builder_.get_content_type(get_extension()(filepath_)));
			http_builder_.append_entry("Content-Length", std::to_string(end - begin).c_str());
			http_builder_.append_entry("Content-Disposition", "attachment; filename=" + get_filename()(filepath_));
			if (has_range)
			{
				std::string content_range("bytes ");
				content_range.append(std::to_string(begin));
				content_range.append("-");
				content_range.append(std::to_string(end));
				content_range.append("/");
				content_range.append(std::to_string(size));

				http_builder_.set_status(206);
				http_builder_.append_entry("Accept-Range","bytes");
				http_builder_.append_entry("Content-Range",content_range);
			}
			
			std::string buffer;
			buffer.resize(102400);
			std::function<void()> resume_handle;
			xnet::connection &conn = request_.conn_;
			file_.seekg(begin, std::ios::beg);
			conn.regist_send_callback([&](std::size_t len) {
				if (len == 0)
				{
					close();
					file_.close();
					resume_handle();
					return;
				}
				if (begin == end)
				{
					file_.close();
					resume_handle();
					return;
				}
				auto to_reads = std::min<uint64_t>(buffer.size(), end - begin);
				file_.read((char*)buffer.data(), to_reads);
				auto gcount = file_.gcount();
				if (gcount > 0)
				{
					conn.async_send(buffer.data(), (uint32_t)gcount);
					begin += gcount;
				}
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
