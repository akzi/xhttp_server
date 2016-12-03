#pragma once
namespace xhttp_server
{
	
	class response
	{
	public:
		enum class version
		{
			HTTP_1_0,
			HTTP_1_1
		};
		
		response()
		{

		}
		response &set_version(version v)
		{
			if (v == version::HTTP_1_0)
				builder_.set_version("HTTP/1.0");
			return *this;
		}
		response &set_status(int status)
		{
			builder_.set_status(status);
			return *this;
		}
		response &add_header(const std::string &keyname, const std::string &value)
		{
			builder_.append_header(keyname, value);
			return *this;
		}
		template<typename T>
		response &set_data(T&& buffer)
		{
			data_ = std::forward<T>(buffer);
			return *this;
		}
		void done()
		{
			builder_.append_header("Content-Length", std::to_string(data_.size()).c_str());
			std::string buffer = std::move(builder_.build());
			buffer.append(data_);
			send_buffer_(std::move(buffer));
		}
	private:
		friend class request;
		void reset()
		{
			data_.clear();
			builder_.reset();
		}
		std::function<void(std::string &&)> send_buffer_;
		std::string data_;
		xhttper::http_builder builder_;
	};
}