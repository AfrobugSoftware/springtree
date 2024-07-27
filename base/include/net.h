#pragma once
//pharmaoffice uses http for communication with the outside world
//it seralised tuples as j

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <boost/noncopyable.hpp>

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <fmt/format.h>


#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>

#include <boost/signals2/signal.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/lockfree_forward.hpp> //lets see what you can give us
#include <boost/crc.hpp>


#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <sstream>
#include <chrono>
#include <algorithm>

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <future>
#include <type_traits>
#include <atomic>
#include <utility>

#include "taskmanager.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using net::as_tuple_t;
using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable_t;
using default_token = as_tuple_t<use_awaitable_t<>>;

using executor_with_default = default_token::executor_with_default<net::any_io_executor>;
using tcp_stream = typename beast::tcp_stream::rebind_executor<executor_with_default>::other;
using namespace boost::asio::experimental::awaitable_operators;


extern boost::asio::ip::tcp::endpoint m_globalendpoint;
namespace this_coro = net::this_coro;


using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;
#define PHARMAOFFICE_USER_AGENT_STRING "springtree"

namespace pof {
	namespace base {
		namespace js = nlohmann;
		using timer_t = default_token::as_default_on_t<boost::asio::steady_timer>;
		
		namespace ssl {
			template<typename resp_body = beast::http::string_body, typename req_body = boost::beast::http::empty_body>
			class session : public std::enable_shared_from_this<session<resp_body, req_body>> , public boost::noncopyable {
			public:
				using req_body_t = typename req_body::value_type;
				using resp_body_t = typename resp_body::value_type;
				using request_type = http::request<req_body>;
				using response_type = http::response<resp_body>;
				using promise_t = std::promise<response_type>;
				using future_t = std::future<response_type>;

				std::atomic_bool m_connected;
				session(boost::asio::io_context& ios, boost::asio::ssl::context& ssl) :
					m_io{ios},
					m_ctx{ssl},
					m_resolver{net::make_strand(ios.get_executor())}
				, m_stream{ net::make_strand(ios.get_executor()), ssl }, m_connected{false} {
						
				}

				future_t req( http::verb v, 
					const std::string& target,
					typename request_type::body_type::value_type&& body,
					const std::string& host = ""s,
					const std::string& port = ""s,
					std::chrono::steady_clock::duration dur = 60s) {
					//prepare the request
					m_dur = dur;
					prepare_request(host, target, v, body);
					if (!host.empty() && !port.empty()) {
						tcp::resolver::query q{ host, port };
						m_resolver.async_resolve(q, beast::bind_front_handler(&session::on_resolve, this->shared_from_this()));
					}
					else {
						if (m_connected.load()) {
							boost::asio::co_spawn(m_stream.get_executor(), run(), [this_ = this->shared_from_this()](std::exception_ptr eptr, response_type res) {
								if (eptr) this_->m_promise.set_exception(eptr);
								else this_->m_promise.set_value(res);
							});
						}
						else {
							boost::asio::co_spawn(m_stream.get_executor(), run(m_globalendpoint), [this_ = this->shared_from_this()](std::exception_ptr eptr, response_type res) {
								if (eptr) this_->m_promise.set_exception(eptr);
								else this_->m_promise.set_value(res);
							});
						}
					}

					return (m_promise.get_future());
				}

				void cancel() {
					m_connected.store(false);
					beast::get_lowest_layer(m_stream).socket().close();
				}

				void on_fail(std::error_code code)
				{
					const int err = code.value();
					if (err == net::error::eof
						|| err == net::error::basic_errors::operation_aborted 
						|| err == net::ssl::error::stream_truncated) { //ignore stream truncated error
						return; 
					}
					else if (err == boost::asio::error::not_connected || err == boost::asio::error::connection_aborted ||
						err == boost::asio::error::connection_reset || err == boost::asio::error::connection_refused) {
						m_connected.store(false);
					}
					else if (boost::system::error_code(code) == http::error::end_of_stream) {
						m_stream.shutdown();
					}

					spdlog::error("Error: {}", code.message());
					throw std::system_error(code);
				}


				void on_resolve(std::error_code code, tcp::resolver::results_type results) {
					if (code) {
						on_fail(code);
					}
					co_spawn(m_stream.get_executor(), run(std::move(results)), [&](std::exception_ptr ptr, response_type resp) {
							if (ptr) m_promise.set_exception(ptr);
							else m_promise.set_value(resp);
					});
				}
				awaitable<response_type> run(tcp::resolver::results_type results)
				{
					//hold an instance of the resource
					auto this_ = this->shared_from_this();

					//get the socket
					auto& sock = beast::get_lowest_layer(m_stream).socket();
					std::error_code ec{};
					size_t bytes = 0;
					tcp::resolver::results_type::endpoint_type ep{};
			
					//connect
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					
					std::tie(ec, ep) = co_await net::async_connect(sock, results);
					if (ec) {
						on_fail(ec);
					}
					//handshake 
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
				
					std::tie(ec) = co_await m_stream.async_handshake(net::ssl::stream_base::client);
					if (ec) {
						on_fail(ec);
					}

					//write
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_write(m_stream, m_req);
					if (ec) {
						on_fail(ec);
					}
				
					//read
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_read(m_stream, m_buf, m_resp);
					if (ec) {
						on_fail(ec);
					}

					co_return m_resp;
				}

				awaitable<response_type> run(boost::asio::ip::tcp::endpoint ep)
				{
					auto& sock = beast::get_lowest_layer(m_stream).socket();
					std::error_code ec{};
					size_t bytes = 0;
					

					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::chrono::system_clock::time_point starttime = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point stoptime;

					//connect
					std::tie(ec) = co_await sock.async_connect(ep, net::as_tuple(net::use_awaitable));
					if (ec) {
						on_fail(ec);
					}

					//handshake 
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec) = co_await m_stream.async_handshake(net::ssl::stream_base::client);
					if (ec) {
						on_fail(ec);
					}

					//write
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_write(m_stream, m_req);
					if (ec) {
						on_fail(ec);
					}

					//read
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_read(m_stream, m_buf, m_resp);
					if (ec) {
						on_fail(ec);
					}
					
					co_return m_resp;
				}

				awaitable<response_type> run()
				{
					//write
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					auto&&[ec, bytes] = co_await http::async_write(m_stream, m_req);
					if (ec) {
						on_fail(ec);
					}

					//read
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_read(m_stream, m_buf, m_resp);
					if (ec) {
						on_fail(ec);
					}

					co_return m_resp;
				}

				void close() {
					m_connected.store(false);
					boost::asio::co_spawn(m_stream.get_executor(), m_stream.async_shutdown(net::use_awaitable)
						, boost::asio::detached);
				}

				//PREPARE THE REQUEST 
				void prepare_request(const std::string& host,
					const std::string& target,
					http::verb verb,
					req_body_t const& body,
					int version = 11
				)
				{
					if constexpr (std::is_same_v<req_body, http::string_body>) {
						//if not empty body
						//string bodies
						m_req.version(version);
						m_req.method(verb);
						m_req.target(target.c_str());
						m_req.set(http::field::host, host.c_str());
						m_req.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
						m_req.set(http::field::content_length, std::to_string(body.size()));

						m_req.body() = body;
						m_req.prepare_payload();
					}
					else if constexpr (std::is_same_v<req_body, http::file_body>) {
						//if request is a file body 
						http::request<http::file_body> req_{ std::piecewise_construct,
							std::make_tuple(std::move(body)) };
						req_.method(verb);
						req_.version(version);
						req_.target(target.c_str());
						req_.set(http::field::host, host.c_str());
						req_.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
						req_.set(http::field::content_length, std::to_string(req_.body().size()));
						m_req = std::move(req_);
						m_req.prepare_payload();

					}
					else if constexpr (std::is_same_v<req_body, http::empty_body>) {
						// the body is empty, usual get request have empty bodies
						m_req.version(version);
						m_req.method(verb);
						m_req.target(target.c_str());
						m_req.set(http::field::host, host.c_str());
						m_req.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
					}
					else if constexpr (std::is_same_v<req_body, http::vector_body<std::uint8_t>>) {
						m_req.version(version);
						m_req.method(verb);
						m_req.target(target.c_str());
						m_req.set(http::field::host, host.c_str());
						m_req.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
						m_req.set(http::field::content_type, "application/octlet-stream"s);
						m_req.set(http::field::content_length, std::to_string(body.size()));

						m_req.body() = std::move(body);
						m_req.prepare_payload();

					}
					//ignore all other bodies for now
				}

				net::io_context& m_io;
				net::ssl::context& m_ctx;
				tcp::resolver m_resolver;


				beast::ssl_stream<tcp_stream> m_stream;
				beast::flat_buffer m_buf;
				request_type m_req;
				response_type m_resp;
				std::chrono::steady_clock::duration m_dur;

				promise_t m_promise;

			};
		}
	}
}

