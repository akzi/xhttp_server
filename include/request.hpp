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
			if (!body_len_)
			{
				std::string len = parser_.
					get_header<strncasecmper>("Content-Length");

				if (len.empty())
				{
					body_len_ = 0;
					return body_len_;
				}
				body_len_ = std::strtoul(len.c_str(), 0, 10);
			}
			return body_len_;
		}
		std::string body()
		{
			std::string buffer_ = parser_.get_string();
			if (buffer_.size() == content_length())
				return buffer_;
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
					buffer_.append(data, len);
					if (buffer_.size() == content_length())
					{
						resume_handle();
						return;
					}
					conn_.async_recv_some();
				});
				conn_.async_recv_some();
				xcoroutine::yield(resume_handle);
			}
			return buffer_;
		}
		bool keepalive()
		{
			if (keepalive_ == -1)
			{
				std::string conn = parser_.
					get_header<strncasecmper>("Connection");
				if (conn.empty())
				{
					keepalive_ = 0;
					return false;
				}
				if (strcasecmper()(conn.c_str(), "Keep-Alive"))
				{
					keepalive_ = 1;
				}
			}
			return !!keepalive_;
		}
		xsession get_session()
		{
			std::string session_id = parser_.get_header<strncasecmper>("XSEESSIONID");
			if (session_id.empty())
				session_id = gen_session_id();
			assert(proactor_);
			return xsession(detail::redis_creater::get_instance().
				get_redis(*proactor_), session_id);
		}
		std::string get_header(const std::string &name)
		{
			return parser_.get_header<strncasecmper>(name.c_str());
		}
	private:
		friend class xserver;
		friend class file_uploader;
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
			conn_.close();
			close_callback_();
		}
		void init()
		{
			resp.send_buffer_ = [this](std::string &&buffer) 
			{
				send_buffers_.emplace_back(std::move(buffer));
				try_send();
				parser_.reset();
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
			if (is_send_)
				return;
			if (send_buffers_.empty()) 
			{
				is_send_ = false;
				return;
			}
			conn_.async_send(std::move(send_buffers_.front()));
			send_buffers_.pop_front();
		}
		bool is_send_ = false;
		std::function<void()> close_callback_;
		std::function<void()> handle_request_;
		std::function<void(std::string &&)> body_callback_;
		std::string body_;
		xnet::connection conn_;
		std::size_t body_len_ = 0;
		int keepalive_ = -1;
		method method_ = NUll;
		xhttper::parser parser_;
		std::list<std::string> send_buffers_;
		response resp;
		xnet::proactor *proactor_ = nullptr;
	};
}