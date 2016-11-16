#pragma once
#include "common.hpp"
#include "request.hpp"
#include "response.hpp"
namespace xhttp_server
{
	class xserver
	{
	public:
		using request_handler = std::function<void(request &req, response &rsp)>;
		xserver()
			:acceptor_(std::move(proactor_.get_acceptor()))
		{
		}
		void bind(const std::string &ip, int port)
		{
			acceptor_.bind(ip, port);
			acceptor_.regist_accept_callback(
				[this](xnet::connection &&conn) {
				accept_callback(std::move(conn));
			});
		}
		void run()
		{
			proactor_.run();
		}
		void stop()
		{
			proactor_.stop();
		}
		void regist(request_handler handle)
		{
			request_handler_ = handle;
		}
	private:
		void accept_callback(xnet::connection &&conn)
		{

		}
		request_handler request_handler_;
		xnet::proactor proactor_;
		xnet::acceptor acceptor_;
	};
}