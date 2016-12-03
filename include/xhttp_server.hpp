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
		void start(std::size_t thread_count = std::thread::hardware_concurrency())
		{
			proactor_pool_.set_size(thread_count);
			proactor_pool_.regist_run_before([this] {	before_run(); });
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
		xserver &set_redis_addr(const std::string &ip, int port,bool cluster = false)
		{
			redis_ip_ = ip;
			redis_port_ = port;
			redis_cluster_ = cluster;
			return *this;
		}
	private:
		void before_run()
		{
			if (redis_ip_.empty())
				return;
			auto &redis = detail::redis_creater::get_instance().get_redis(
				proactor_pool_.get_current_proactor());
			redis.set_addr(redis_ip_, redis_port_, redis_cluster_);
			if (redis_cluster_)
			{
				redis.regist_cluster_init_callback([](
					std::string &&error_code, bool status) {
					if (status)
					{
						std::cout << "thread: " 
							<< std::this_thread::get_id() 
							<< " redis cluster init ok" 
							<< std::endl;
					}
				});
			}
			else {
				redis.regist_connect_success_callback([] {
					std::cout << "thread: " 
						<< std::this_thread::get_id() 
						<< " redis connect ok" << std::endl;
				});
				redis.regist_connect_failed_callback([](const std::string &error_code) {
					std::cout << "thread: "
						<< std::this_thread::get_id()
						<< " redis connect failed: " << error_code << std::endl;
				});
			}
		}
		void accept_callback(xnet::connection &&conn)
		{
			auto req = std::make_shared<request>();
			req->conn_ = std::move(conn);
			req->init();
			req->handle_request_ = std::bind(&xserver::handle_request, 
				this, std::ref(*req));
			auto id = gen_id();
			req->id_ = id;
			req->close_callback_ = [&,id] {
				remove_request(id);
			};
			req->proactor_ = &proactor_pool_.get_current_proactor();
			add_request(req->id_, std::move(req));
		}
		void handle_request(request &req)
		{
			xcoroutine::create([&] {
				req.reset();
				req.in_callback_ = true;
				request_handler_(req, req.resp_);
				req.in_callback_ = false;
				req.parser_.reset();
				if (req.is_close_)
					remove_request(req.id_);
			});
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
		std::string redis_ip_;
		int redis_port_ = 6379;
		bool redis_cluster_ = false;
	};
}