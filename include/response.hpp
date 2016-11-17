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
			buffer_ = std::forward<T>(buffer);
			return *this;
		}
	private:
		std::string buffer_;
		xhttper::builder builder_;
	};
}