#pragma once
#include "common.hpp"
#include "response.hpp"
#include "request.hpp"
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
			auto req = std::make_shared<request>();
			req->conn_ = std::move(conn);
			req->init();
			req->handle_request_ = std::bind( 
				&xserver::handle_request, this, std::ref(*req));
			auto id = gen_id();
			req->close_callback_ = [id,this]{
				remove_request(id);
			};
			requests_.emplace(id, req);
		}
		void handle_request(request &req)
		{
			request_handler_(req, req.resp);
		}
		void remove_request(int64_t id)
		{
			requests_.erase(requests_.find(id));
		}
		int64_t gen_id()
		{
			return ++id_;
		}
		int64_t id_ = 0;
		std::map<int64_t, std::shared_ptr<request>> requests_;
		request_handler request_handler_;
		xnet::proactor proactor_;
		xnet::acceptor acceptor_;
	};
}