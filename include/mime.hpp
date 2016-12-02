#pragma once
namespace xhttp_server
{
	class mime_parser
	{
	public:
		struct disposition
		{
			std::string name_;
			std::unique_ptr<std::string> filename_;
		};
		mime_parser()
		{

		}
		bool do_parser(char *data, std::size_t len)
		{
			if (!do_parse_boundary(data, len))
				return false;
			if (len && !do_parse_headers(data, len))
				return false;
			if (do_recv_data(data, len))
			{

			}
			return true;
		}
	private:
		bool do_recv_data(char *&data, std::size_t &len)
		{
			do 
			{
				if (!find_hyphen_)
				{
					if (get_string<'-'>(buffer_, data, len))
					{
						find_hyphen_ = true;
					}
				}
				if (find_hyphen_ && len)
				{
					try
					{
						if (do_parse_boundary(data, len))
						{
							file_data_callback();
							return true;
						}
					}
					catch (const std::exception& e)
					{
						buffer_.append(boundary_buffer_);
					}
				}
			} while (len);
			file_data_callback();
		}
		bool do_parse_headers(char *&data, std::size_t &len)
		{
			do 
			{
				if (len && !do_parse_header_name(data, len))
					return false;
				if (len && !do_parse_header_data(data, len))
					return false;
				if (len &&!get_CR1(data, len))
					return false;
				if (len && !get_CF1(data, len))
					return false;
				if (check_headers_end(data))
				{
					if (len && !get_CR2(data, len))
						return false;
					if (len && !get_CF2(data, len))
						return false;
					return true;
				}
			} while (true);
		}
		bool get_CR1(char *&data, std::size_t &len)
		{
			if (cr1_done_)
				return true;
			if (get_char<'\r'>(data, len))
			{
				cr1_done_ = true;
				return true;
			}
			return false;
		}
		bool get_CR2(char *&data, std::size_t &len)
		{
			if (cr2_done_)
				return true;
			if (get_char<'\r'>(data, len))
			{
				cr2_done_ = true;
				return true;
			}
			return false;
		}
		bool get_CF1(char *&data, std::size_t &len)
		{
			if (cf1_done_)
				return true;
			if (get_char<'\r'>(data, len))
			{
				cf1_done_ = true;
				return true;
			}
			return false;
		}
		bool get_CF2(char *&data, std::size_t &len)
		{
			if (cf2_done_)
				return true;
			if (get_char<'\r'>(data, len))
			{
				cf2_done_ = true;
				return true;
			}
			return false;
		}
		bool check_headers_end(char *data)
		{
			if (check_headers_end_)
				return true;
			if (data[0] == '\r')
			{
				check_headers_end_ = true;
				return true;
			}
			reset();
			return false;
		}
		template<char Char>
		bool get_char(char *&data, std::size_t &len)
		{
			if (data[0] == Char)
			{
				cr1_done_ = true;
				data += 1;
				len -= 1;
				return true;
			}
			return false;
		}
		template<char terminater>
		bool get_string(std::string &buffer, char *&data, std::size_t &len)
		{
			int i = 0;
			while (data[i] != terminater && i < len)
			{
				header_data_.push_back(data[i]);
				++i;
			}
			if (data[i] == terminater)
			{
				len -= i;
				data += i;
				return true;
			}
			return false;
		}
		bool do_parse_header_data(char *&data, std::size_t &len)
		{
			if (header_data_done_)
				return true;
			if (get_string<'\r'>(header_data_, data, len))
			{
				header_data_done_ = true;
				return true;
			}
			return false;
		}
		bool do_parse_header_name(char *&data, std::size_t &len)
		{
			if (header_name_done_)
				return true;
			if (get_string<':'>(header_name_, data, len))
			{
				header_name_done_ = true;
				return true;
			}
			return false;
		}
		bool do_parse_boundary(char *&data, std::size_t &len)
		{
			if (boundary_buffer_.size() < boundary_.size())
			{
				auto min_len = std::min<std::size_t>(boundary_.size() - boundary_buffer_.size(), len);
				boundary_buffer_.append(data, min_len);
				data += min_len;
				len += min_len;
				if (len)
				{
					if(boundary_buffer_ == boundary_)
						return true;
					throw std::runtime_error("boundary error:"+boundary_buffer_);
				}
				return false;
			}
			return true;
		}
		void file_data_callback()
		{

		}
		void reset()
		{
			header_data_.clear();
			header_data_done_ = false;
			header_name_.clear();
			header_name_done_ = false;
			cr1_done_ = false;
			cr2_done_ = false;
			cf1_done_ = false;
			cf2_done_ = false;
		}
		bool find_hyphen_ = false;
		std::string buffer_;

		bool check_headers_end_ = false;
		bool cr1_done_ = false;
		bool cf1_done_ = false;
		bool cr2_done_ = false;
		bool cf2_done_ = false;
		bool header_name_done_ = false;
		std::string header_name_;
		bool header_data_done_ = false;
		std::string header_data_;
		std::map<std::string, std::string> entries_;
		std::size_t boundary_length_;
		std::string boundary_;
		std::string boundary_buffer_;
		std::size_t content_length_ = 0;
	};

}