#pragma once
namespace xhttp_server
{
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
		int body_len()
		{
			if (body_len_ == -1)
			{
				std::string len = parser_.get_header("Content-Length");
				if (len.empty())
				{
					body_len_ = 0;
					return body_len_;
				}
				body_len_ = std::strtol(len.c_str(), 0, 10);
			}
			return body_len_;
		}
		void get_body(std::function<void(std::string &&)> &&callback)
		{
			int len = body_len();
			if (len == 0)
			{
				callback({});
				return;
			}
			int remain = parser_.remain_len();
			if (len <= remain)
			{
				callback(parser_.get_string(len));
				return;
			}
			body_ = std::move(parser_.get_string());
			conn_.async_recv(len);
		}
	private:
		friend class xserver;
		void recv_callback(char *data, int len)
		{
			if (len <= 0)
				return close();
			if (body_callback_)
			{
				int remain = body_len_ - body_.size();
				body_.append(data, remain);
				data += remain;
				len -= remain;
				body_callback_(std::move(body_));
				body_callback_ = nullptr;
			}
			parser_.reset();
			if (len)
			{
				parser_.append(data, len);
				if (parser_.parse_req())
					handle_request_();
			}
			conn_.async_recv_some();
		}
		void close()
		{

		}
		void init()
		{

		}
		std::function<void()> handle_request_;
		std::function<void(std::string &&)> body_callback_;
		std::string body_;
		xnet::connection conn_;
		int body_len_ = -1;
		method method_ = NUll;
		xhttper::parser parser_;
		std::list<std::string> send_buffers_;
	};
}