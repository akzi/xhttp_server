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
			if (status == 404)
			{
				data_ = "<html><body><h1>404</1h></body></html>";
			}
			return *this;
		}
		response &add_entry(const std::string &keyname, const std::string &value)
		{
			builder_.append_entry(keyname, value);
			return *this;
		}
		template<typename T>
		response &set_data(T&& buffer)
		{
			data_ = std::forward<T>(buffer);
			return *this;
		}
		bool send_file(const std::string &filepath)
		{
			using get_extension = xutil::functional::get_extension;

			std::ifstream file(filepath, std::ifstream::binary);
			if (!file)
				return false;
			std::string buffer((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());
			data_ = std::move(buffer);
			file.close();
			builder_.append_entry("Content-Type", builder_.get_content_type(get_extension()(filepath)));
			return true;
		}
		void done()
		{
			if(data_.size())
				builder_.append_entry("Content-Length", std::to_string(data_.size()));
			std::string buffer = builder_.build_resp();
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