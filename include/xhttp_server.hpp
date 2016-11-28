#pragma once
#include "detail\detail.hpp"
namespace xhttp_server
{
	class xserver
	{
	public:
		using request_handler = std::function<void(request &req, response &rsp)>;
		xserver()
		{
		}
		xserver &bind(const std::string &ip, int port)
		{
			proactor_pool_.bind(ip, port);
			proactor_pool_.regist_accept_callback(
				[this](xnet::connection &&conn) {
				accept_callback(std::move(conn));
			});
			return *this;
		}
		void start()
		{
			proactor_pool_.start();
		}
		void stop()
		{
			proactor_pool_.stop();
		}
		xserver &regist(request_handler handle)
		{
			request_handler_ = handle;
			return *this;
		}
	private:
		void accept_callback(xnet::connection &&conn)
		{
			auto req = std::make_shared<request>();
			req->conn_ = std::move(conn);
			req->init();
			req->handle_request_ = std::bind(&xserver::handle_request, 
				this, std::ref(*req));
			auto id = gen_id();
			req->close_callback_ = [id,this]{
				remove_request(id);
			};
			req->proactor_ = &proactor_pool_.get_current_proactor();
			add_request(id, std::move(req));
		}
		void handle_request(request &req)
		{
			request_handler_(req, req.resp);
		}
		void add_request(int64_t id, std::shared_ptr<request> && req)
		{
			std::lock_guard<std::mutex> lock(requests_mutex_);
			requests_.emplace(id, req);
		}
		void remove_request(int64_t id)
		{
			std::lock_guard<std::mutex> lock(requests_mutex_);
			requests_.erase(requests_.find(id));
		}
		int64_t gen_id()
		{
			return ++id_;
		}
		int64_t id_ = 0;
		std::mutex requests_mutex_;
		std::map<int64_t, std::shared_ptr<request>> requests_;
		request_handler request_handler_;
		xnet::proactor_pool proactor_pool_;
	};
}