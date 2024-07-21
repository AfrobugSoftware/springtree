#pragma once
#include "net.h"
#include "errc.h"
#include <boost/asio/signal_set.hpp>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/url/urls.hpp>
#include <boost/url.hpp>

#include "router.hpp"

#include <thread>
#include <mutex>
#include <unordered_map>
#include <functional>

namespace pof
{
	namespace base {
		class net_manager : private boost::noncopyable
		{
		public:
			using req_t = http::request<http::vector_body<std::uint8_t>>;
			using res_t = http::response<http::vector_body<std::uint8_t>>;
			using callback = std::function<boost::asio::awaitable<res_t>(req_t&&, boost::urls::matches&&)>;
			class httpsession : public boost::enable_shared_from_this<httpsession>
			{
				net_manager& manager;
				beast::tcp_stream stream_;
				beast::flat_buffer buffer_;

				boost::optional<http::request_parser<http::vector_body<std::uint8_t>>> parser;

				void fail(beast::error_code ec, char const* what);
				void do_read();
				void on_read(beast::error_code ec, std::size_t);
				void on_write(beast::error_code ec, std::size_t, bool close);
			public:
				httpsession(net_manager& man, tcp::socket&& socket);
				void run();
			};


			class listener : public boost::enable_shared_from_this<listener>
			{
				net_manager& manager;
				net::io_context& ioc_;
				tcp::acceptor acceptor_;
				//boost::shared_ptr<shared_state> state_;

				void fail(beast::error_code ec, char const* what);
				void on_accept(beast::error_code ec, tcp::socket socket);

			public:
				listener(net_manager& man, net::io_context& ioc, tcp::endpoint endpoint);
				void run();
			};

			net_manager();
			net_manager(net_manager&& manage) = delete;
			net_manager& operator=(net_manager&& manage) = delete;

			bool stop();
			std::error_code setupssl();

			inline net::io_context& io() { return m_io; }
			inline net::ssl::context& ssl() { return m_ssl; }


			res_t bad_request(const std::string& err) const;
			res_t server_error(const std::string& err) const;
			res_t not_found(const std::string& err) const;
			res_t auth_error(const std::string& err) const;
			res_t unprocessiable(const std::string& err) const;
			res_t timeout_error() const;
		
			void run();

			inline void bind_addr(boost::asio::ip::tcp::endpoint ep) { m_endpoint = ep; };
			void add_route(const std::string& target, callback&& cb);

			friend class httpsession;
		private:
			std::unique_ptr<net::executor_work_guard<boost::asio::io_context::executor_type>> m_workgaurd;
			std::shared_ptr<listener> m_listener;
			std::shared_ptr<net::signal_set> m_signals;
			boost::asio::ip::tcp::endpoint m_endpoint;

			boost::urls::router<callback> m_router;
			net::io_context m_io;
			net::ssl::context m_ssl;
			std::thread m_thread;

			std::vector<std::thread> m_threadvec;
		};
	};
};