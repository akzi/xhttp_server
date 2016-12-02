#pragma once
namespace xhttp_server
{
	class mime
	{
	public:
		mime()
		{

		}
		std::map<std::string, std::string> params_;
		std::map<std::string, std::string> files_;
	};

}