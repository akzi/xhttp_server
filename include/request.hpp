#pragma once
namespace xhttp_server
{
	using namespace xutil::functional;
	class request
	{
	public:
		enum method
		{
			NUll,
			GET,
			POST,
		};
		request()
		{
		}
		method get_method()
		{
			return method_;
		}
		std::size_t content_length()
		{
			if (content_length_ == (std::size_t)-1)
			{
				std::string len = parser_.
					get_header<strncasecmper>("Content-Length");

				if (len.empty())
				{
					content_length_ = 0;
					return content_length_;
				}
				content_length_ = std::strtoul(len.c_str(), 0, 10);
			}
			return content_length_;
		}
		std::string body()
		{
			int i = 0;
			body_ = parser_.get_string();
			if (body_.size() == content_length())
				return body_;
			else
			{
				std::function<void()> resume_handle;
				conn_.regist_recv_callback([&](char *data, std::size_t len)
				{
					if (len == 0) 
					{
						conn_.close();
						resume_handle();
						return;
					}
					i++;
					body_.append(data, len);
					std::cout << body_ << std::endl;
					if (body_.size() == content_length())
					{
						resume_handle();
						return;
					}
					conn_.async_recv_some();
				});
				conn_.async_recv_some();
				xcoroutine::yield(resume_handle);
				do_receive();
			}
			return body_;
		}
		bool keepalive()
		{
			if (keepalive_ == -1)
			{
				std::string buffer = parser_.get_header<strncasecmper>("Connection");
				if (buffer.empty())
				{
					keepalive_ = 0;
					return false;
				}
				if (strncasecmper()(buffer.c_str(), "keep-alive", buffer.size()))
				{
					keepalive_ = 1;
				}
			}
			return !!keepalive_;
		}
		std::string get_boundary()
		{
			if (boundary_.size())
				return boundary_;
			std::string content_type = parser_.get_header<strncasecmper>("Content-Type");
			if (content_type.empty())
				return {};
			if (content_type.find("multipart/form-data") == std::string::npos)
				return {};
			auto pos = content_type.find("boundary=");
			if (pos == std::string::npos)
				return {};
			pos += strlen("boundary=");
			boundary_ = content_type.substr(pos, content_type.size() - pos);
			return boundary_;
		}
		xsession get_session()
		{
			std::string session_id = parser_.get_header<strncasecmper>("XSEESSIONID");
			if (session_id.empty())
				session_id = gen_session_id();
			return{ detail::redis_creater::get_instance().get_redis(), session_id };
		}
		std::string get_header(const std::string &name)
		{
			return parser_.get_header<strncasecmper>(name.c_str());
		}
		std::string req_path()
		{
			return parser_.get_path();
		}
		class xserver &get_xserver()
		{
			return *xserver_;
		}
	private:
		friend class xserver;
		friend class uploader;
		friend class downloader;
		void reset()
		{
			body_.clear();
			boundary_.clear();
			is_close_ = false;
			content_length_ = (std::size_t)-1;
			keepalive_ = -1;
			method_ = NUll;
			resp_.reset();
			parser_.reset();
		}
		std::string gen_session_id()
		{
			return std::to_string(
				std::chrono::high_resolution_clock::now().
				time_since_epoch().
				count());
		}
		void recv_callback(char *data, std::size_t len)
		{
			parser_.append(data, len);
			if (parser_.parse_req())
			{
				handle_request_();
				return;
			}
			conn_.async_recv_some();
		}
		void close()
		{
			is_close_ = true;
			conn_.close();
			if (in_callback_ == false)
				close_callback_();
		}
		void do_receive()
		{
			resp_.send_buffer_ = [this](std::string &&buffer) 
			{
				send_buffers_.emplace_back(std::move(buffer));
				try_send();
			};
			conn_.regist_send_callback([this](std::size_t len) 
			{
				if (len == 0)
				{
					close();
					return;
				}
				try_send();
			});
			conn_.regist_recv_callback([this](char *data, std::size_t len)
			{
				if (len == 0)
				{
					close();
					return;
				}
				recv_callback(data, len);
			});
			conn_.async_recv_some();
		}
		void try_send()
		{
			if (is_send_ || is_close_)
				return;
			if (send_buffers_.empty()) 
			{
				is_send_ = false;
				return;
			}
			conn_.async_send(std::move(send_buffers_.front()));
			send_buffers_.pop_front();
		}
		std::string boundary_;
		bool is_close_ = false;
		bool is_send_ = false;
		int64_t id_ = 0;
		bool in_callback_ = false;
		std::function<void()> close_callback_;
		std::function<void()> handle_request_;
		std::function<void(std::string &&)> body_callback_;
		std::string body_;
		xnet::connection conn_;
		std::size_t content_length_ = (std::size_t)-1;
		int keepalive_ = -1;
		method method_ = NUll;
		xhttper::http_parser parser_;
		std::list<std::string> send_buffers_;
		response resp_;
		xserver *xserver_;
	};
}
