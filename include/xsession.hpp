#pragma once
namespace xhttp_server
{
	class xsession
	{
	public:
		xsession(xredis::redis &redis, const std::string &session_id)
			:redis_(redis),
			session_id_(session_id)
		{

		}
		bool get(const std::string &key, std::string &value)
		{
			bool res = false;
			using xutil::to_function;
			using xcoroutine::apply;
			xredis::hash hash(redis_);
			std::function<void(const std::string &,
				std::string &&, 
				xredis::bulk_callback&&)> func =
				std::bind(&xredis::hash::hget, 
					std::ref(hash), 
					std::placeholders::_1, 
					std::placeholders::_2, 
					std::placeholders::_3);
			auto result = apply(
				to_function(func), 
				session_id_, 
				std::string(key));
			if (std::get<0>(result).empty())
				res = true;
			value = std::get<1>(result);
			return res;
		}
		bool set(const std::string &key, std::string &&value)
		{
			bool res = false;
			using xutil::to_function;
			using xcoroutine::apply;
			xredis::hash hash(redis_);
			std::function<void(const std::string &, 
				std::string &&, 
				std::string &&, 
				xredis::integral_callback&&)> 
				func = std::bind(&xredis::hash::hset, 
					std::ref(hash), 
					std::placeholders::_1, 
					std::placeholders::_2, 
					std::placeholders::_3, 
					std::placeholders::_4);
			auto result = apply(
				to_function(func),
				session_id_, 
				std::string(key), 
				std::move(value));
			if (std::get<0>(result).empty())
				res = true;
			return res;
		}
		void del(const std::string &key)
		{

		}
	private:
		std::string session_id_;
		xredis::redis &redis_;
	};
}