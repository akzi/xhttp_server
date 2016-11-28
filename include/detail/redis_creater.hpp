#pragma once
namespace xhttp_server
{
	namespace detail
	{
		struct redis_creater
		{
		public:
			static redis_creater &get_instance()
			{
				static redis_creater inst;
				return inst;
			}
			xredis::redis &get_redis(xnet::proactor &proactor)
			{
				static thread_local xredis::redis redis(proactor);
				return redis;
			}
		};
	}
}