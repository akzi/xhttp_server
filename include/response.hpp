#pragma once
namespace xhttp_server
{
	class response
	{
	public:
		response()
		{

		}
	private:
		std::shared_ptr<xnet::connection> conn_;
	};
}