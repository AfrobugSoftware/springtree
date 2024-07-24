#pragma once
//pharmaoffice uses http for communication with the outside world
//it seralised tuples as j

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

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


namespace this_coro = net::this_coro;


using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;
#define PHARMAOFFICE_USER_AGENT_STRING "pharmaoffice_1"

namespace pof {
	namespace base {
		namespace js = nlohmann;
		//the session is a single request and reponse cycle 
		//should runs on the thread or threads that call one of the ios
		//TODO: create an sslsession for session over ssl
		using timer_t = default_token::as_default_on_t<boost::asio::steady_timer>;
		template<class response_body, class request_body = http::empty_body>
		class session : public std::enable_shared_from_this<session<response_body, request_body>>
		{
		public:
			static_assert(std::conjunction_v<http::is_body<response_body>, http::is_body<request_body>>,
				"response body or request body is not supported by the session class");

			typedef http::request<request_body>   request_type;
			typedef http::response<response_body> response_type;
			typedef std::promise<response_type>   promise_t;
			typedef std::future<response_type>    future_t;
			static boost::asio::ip::tcp::endpoint m_globalendpoint;
			std::atomic<bool> m_connected;

			explicit session(net::io_context& ioc) : m_resolver(net::make_strand(ioc)),
				m_stream(net::make_strand(ioc))
			{
				//set the maximum size of the read flat_buffer, to avoid buffer overflow
				//catch buffer_overflow errors when this maximum is exceeeded, 
				//but how do i know the maximum buffer size for both the input sequence and the output sequence
				m_connected.store(false);
			}

			session(session&& rhs) = delete;
			session& operator=(session&& rhs) = delete;


			//the copy constructor and the copy assignment is deleted, to prevent copying
			session(const session&) = delete;
			session& operator=(const session&) = delete;

			~session() {}


			template<http::verb verb>
			future_t req(
				const std::string& target,
				typename request_type::body_type::value_type&& body,
				const std::string& host = "",
				const std::string& port = "",
				std::chrono::steady_clock::duration = 60s
			) {
				prepare_request(target, host, verb, std::forward<typename request_type::body_type::value_type>(body), 11);
				if (!host.empty() && !port.empty()){
					m_resolver.async_resolve(host, port, std::bind_front(&session::on_resolve, this->shared_from_this()));
				}
				else {
					if (m_connected.load()) {
						m_stream.async_connect(m_globalendpoint, beast::bind_front_handler(&session::on_connect, this->shared_from_this()));
					}
					else {
						// Send the HTTP request to the remote host
						http::async_write(m_stream,
							m_req, beast::bind_front_handler(&session::on_write,
								this->shared_from_this()));
					}
				}
				return (m_promise.get_future());
			}

			void cancel() {
				m_stream.socket().close();
			}

		private:
			//call backs for async functions
			void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
				if (ec)
					return on_fail(ec);

				// Set a timeout on the operation
				m_stream.expires_after(std::chrono::seconds(60));

				// Make the connection on the IP address we get from a lookup
				m_stream.async_connect(results,
					beast::bind_front_handler(
						&session::on_connect,
						this->shared_from_this()));
			}
			void on_connect(const beast::error_code& ec) {
				if (ec)
					return on_fail(ec);

				// Set a timeout on the operation
				m_stream.expires_after(std::chrono::seconds(60));
				m_connected.store(true);

				// Send the HTTP request to the remote host
				http::async_write(m_stream,
					m_req, beast::bind_front_handler(&session::on_write,
						this->shared_from_this()));
			}
			void on_write(beast::error_code ec, size_t bytes) {
				boost::ignore_unused(bytes);

				if (ec)
					return on_fail(ec);
				
				m_stream.expires_after(std::chrono::seconds(60));

				// Receive the HTTP response
				http::async_read(m_stream, m_buffer, m_res,
					beast::bind_front_handler(
						&session::on_read,
						this->shared_from_this()));
			}

			void on_read(beast::error_code ec, size_t bytes) {
				boost::ignore_unused(bytes);
				// Gracefully close the socket
				m_stream.socket().shutdown(tcp::socket::shutdown_both, ec);

				// not_connected happens sometimes so don't bother reporting it.
				if (ec && ec != beast::errc::not_connected)
					return on_fail(ec);

				//push the result of the respone to the 
				m_promise.set_value(std::move(m_res));

			}

			void on_fail(beast::error_code ec) {
				//causes the future end of the communication channel to throw an exception
				try {
					if (ec == boost::asio::error::not_connected || ec == boost::asio::error::connection_aborted ||
						ec == boost::asio::error::connection_reset || ec == boost::asio::error::connection_refused) {
						m_connected.load(false);
					}
					//throw 
					throw std::system_error(std::error_code(ec));
				}
				catch (...) {
					try {
						m_promise.set_exception(std::current_exception());
					}
					catch (...) {} // do nothing if set_exception throws. mostly writing to a used promise
				}

			}

		private:
			//functions
			void prepare_request(const std::string& target,
				const std::string& host,
				http::verb verb,
				typename request_type::body_type::value_type&& body,
				int version = 11
			)
			{
				if constexpr (std::disjunction_v<std::is_same<request_body, http::string_body>, 
					 std::is_same<request_body, http::vector_body<std::uint8_t>>>) {
					//if not empty body
					//string bodies
					m_req.version(version);
					m_req.method(verb);
					m_req.target(target.c_str());
					m_req.set(http::field::host, host.c_str());
					m_req.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
					m_req.body() = std::forward<typename request_type::body_type::value_type>(body);
				}
				else if constexpr (std::is_same_v<request_body, http::file_body>) {
					//if request is a file body 
					http::request<http::file_body> req_{ std::piecewise_construct,
						std::make_tuple(std::forward<typename request_type::body_type::value_type>(body)) };
					req_.method(verb);
					req_.version(version);
					req_.target(target.c_str());
					req_.set(http::field::host, host.c_str());
					req_.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
					req_.set(http::field::content_length, std::to_string(req_.body().size()));
					m_req = std::move(req_);

				}
				else if constexpr (std::is_same_v<request_body, http::empty_body>) {
					// the body is empty, usual get request have empty bodies
					m_req.version(version);
					m_req.method(verb);
					m_req.target(target.c_str());
					m_req.set(http::field::host, host.c_str());
					m_req.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
				}
				m_req.prepare_payload();
				//ignore all other bodies for now
			}

		private:

			tcp::resolver m_resolver;
			beast::tcp_stream m_stream;

			beast::flat_buffer m_buffer;
			request_type m_req;
			response_type m_res;
			std::chrono::steady_clock::duration mDur;
			//promise
			promise_t m_promise;

		};
		template<typename res_body, typename req_body = http::empty_body>
		using session_weak_ptr = std::weak_ptr<session<res_body, req_body>>;

		template<typename res_body, typename req_body = http::empty_body>
		using session_shared_ptr = std::shared_ptr<session<res_body, req_body>>;

		template<typename res_body, typename req_body>
		boost::asio::ip::tcp::endpoint session<res_body, req_body>::m_globalendpoint =
			 boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8080);

		namespace ssl {
			template<typename resp_body = beast::http::string_body, typename req_body = boost::beast::http::empty_body>
			class session : public std::enable_shared_from_this<session<resp_body, req_body>> {
			public:
				using req_body_t = typename req_body::value_type;
				using resp_body_t = typename resp_body::value_type;
				using req_t = http::request<req_body>;
				using resp_t = http::response<resp_body>;
				using promise_t = std::promise<resp_t>;
				using future_t = std::future<resp_t>;

				session(boost::asio::io_context& ios, boost::asio::ssl::context& ssl) :
					m_io{ios},
					m_ctx{ssl},
					m_resolver{net::make_strand(ios.get_executor())}
					,m_stream{net::make_strand(ios.get_executor()), ssl} {
						
				}

				template<http::verb v>
				future_t req(const std::string& host,
							 const std::string& target,
							 const std::string& port,
							 const req_body_t& rbody = req_body_t{},
							 std::chrono::steady_clock::duration dur = 60s) {
					//prepare the request
					m_dur = dur;
					prepare_request(host, target, v, rbody);
					//co_spawan the run command
					tcp::resolver::query q{ host, port };
					m_resolver.async_resolve(q,
						beast::bind_front_handler(&session::on_resolve, 
							this->shared_from_this()));

					return (m_promise.get_future());
				}

				void cancel() {
					beast::get_lowest_layer(m_stream).socket().close();
				}
			//ASYNC/CALLBACKS FUNCTIONS
			private:
				void on_fail(std::error_code code)
				{
					const int err = code.value();
					if (err == net::error::eof
						|| err == net::error::basic_errors::operation_aborted 
						|| err == net::ssl::error::stream_truncated) { //ignore stream truncated error
						return; 
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
						return;
					}
					co_spawn(m_stream.get_executor(), run(std::move(results)), [&](std::exception_ptr ptr, resp_t resp) {
						try {
							if (ptr) {
								m_promise.set_exception(ptr);
							}
							else {
								m_promise.set_value(resp);
							}
						}catch (std::future_error& exp) {
							//what to do here ?? incase of broken future pipes
						}
					});
				}
				awaitable<resp_t> run(tcp::resolver::results_type results)
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
					std::chrono::system_clock::time_point starttime = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point stoptime;

					spdlog::info("Connecting..");
					std::tie(ec, ep) = co_await net::async_connect(sock, results);
					if (ec) {
						on_fail(ec);
						co_return std::move(m_resp);
					}
					spdlog::info("Connected to {}", ep.address().to_string());
					//handshake 
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					spdlog::info("Creating handshake");
					std::tie(ec) = co_await m_stream.async_handshake(net::ssl::stream_base::client);
					if (ec) {
						on_fail(ec);
						stoptime = std::chrono::system_clock::now();

						co_return std::move(m_resp);
					}
					spdlog::info("Completed");

					//write
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_write(m_stream, m_req);
					if (ec) {
						on_fail(ec);
						co_return std::move(m_resp);
					}
					spdlog::info("Wrote {:d}", bytes);
					std::cout << m_req << std::endl;
					//read
					beast::get_lowest_layer(m_stream).expires_after(m_dur);
					std::tie(ec, bytes) = co_await http::async_read(m_stream, m_buf, m_resp);
					if (ec) {
						on_fail(ec);
						co_return std::move(m_resp);
					}
					spdlog::info("Read {:d}", bytes);
					std::cout << m_resp << std::endl;
					//close
					std::tie(ec) = co_await m_stream.async_shutdown(net::as_tuple(net::use_awaitable));
					if (ec) {
						on_fail(ec);
					}
					co_return std::move(m_resp);
				}

			private:
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
					else if constexpr (std::is_same_v<req_body, http::vector_body>) {
						m_req.version(version);
						m_req.method(verb);
						m_req.target(target.c_str());
						m_req.set(http::field::host, host.c_str());
						m_req.set(http::field::user_agent, PHARMAOFFICE_USER_AGENT_STRING);
						m_req.set(http::field::content_type, "application/bin"s);
						m_req.set(http::field::content_length, std::to_string(body.size()));

						m_req.body() = std::move(body);
						m_req.prepare_payload();

					}
					//ignore all other bodies for now
				}

			private:
				net::io_context& m_io;
				net::ssl::context& m_ctx;
				tcp::resolver m_resolver;


				beast::ssl_stream<tcp_stream> m_stream;
				beast::flat_buffer m_buf;
				req_t m_req;
				resp_t m_resp;
				std::chrono::steady_clock::duration m_dur;

				promise_t m_promise;

			};

			template<typename res_body, typename req_body = http::empty_body>
			using session_weak_ptr = std::weak_ptr<session<res_body, req_body>>;

			template<typename res_body, typename req_body = http::empty_body>
			using session_shared_ptr = std::shared_ptr<session<res_body, req_body>>;


			class wb_session {
			public:
				using clock_t = std::chrono::system_clock;
				using time_rep_t = clock_t::time_point::rep;
				enum : std::uint8_t {
					WB_HELLO,
					WB_DATA,
					WB_CONFIG,
				};
				/**
				*only one slot should process a message, if
				* the slot that services the signal returns true while the rest 
				* returns false, a search down is also halted when done
				*/
				struct combiner {
					using result_type = bool;
					template<typename Iterator>
					bool operator()(Iterator first, Iterator last) {
						while (first != last) {
							if ((*first)) {
								return true;
							}
							first++;
						}
						return false;
					}

				};

				struct wb_message_header {
					std::uint32_t mIdentifier = 0;
					std::uint32_t mType = 0;
					std::uint32_t mMessageLength = 0;
					time_rep_t mTimeStamp = 0;
				};
				constexpr static const size_t wb_message_header_length = sizeof(wb_message_header);
				using wb_message_body = std::vector<std::uint8_t>;
				using wb_message = std::tuple<wb_message_header, wb_message_body>;
				using recieve_signal_t = boost::signals2::signal<bool(std::error_code, const wb_message&), combiner>;

				wb_session(boost::asio::io_context& ios, 
					boost::asio::ssl::context& ssl,
					const std::string& host, const std::string& port, const std::string& path = "/"s) :
					m_stream(boost::asio::make_strand(ios), ssl), m_resolver(boost::asio::make_strand(ios)), m_host{ host }, m_target{path} {
					boost::asio::ip::tcp::resolver::query q{ host, port };
					m_resolver.async_resolve(q, beast::bind_front_handler(&wb_session::on_resolve, this));
					m_stream.control_callback(std::bind_front(&pof::base::ssl::wb_session::control_frame, this));
				}

				~wb_session() {
					m_stream.next_layer().next_layer().close();
				}

				bool write(wb_message&& mes){
					bool is_writing = !m_msg_que.empty();
					bool wrote =  m_msg_que.push(std::forward<wb_message>(mes));
					if (!is_writing && wrote) { //if we were currently not writing and we wrote to the ringbuffer, then spawn
						co_spawn(m_stream.get_executor(), do_write(), [&](std::exception_ptr ptr) {
							if (ptr) {
								std::rethrow_exception(ptr); //is this something ? or capture the exception, print to log and shut down websocket? 
							}
						});
					}
					return wrote;
				}

			protected:
				void on_resolve(std::error_code ec, tcp::resolver::results_type results)
				{
					if (ec) {
						fail(ec);
						return;
					}
					co_spawn(m_stream.get_executor(), 
						run(std::move(results)), 
						[&](std::exception_ptr ptr) {
						if (ptr) {
							std::rethrow_exception(ptr);
						}
						});
				}

				awaitable<void> run(tcp::resolver::results_type results)
				{
					//connect
					m_stream.binary(true); //set binary
					auto [ec, ep] = co_await boost::beast::get_lowest_layer(m_stream).async_connect(std::move(results));
					if (ec) {
						fail(ec);
						co_return;
					}

					//ssl handshake
					beast::get_lowest_layer(m_stream).expires_after(1s);
					spdlog::info("Creating handshake");
					std::tie(ec) = co_await m_stream.next_layer().async_handshake(net::ssl::stream_base::client);
					if (ec) {
						fail(ec);
					}

					//wb socket handshake
					//std::tie(ec) = co_await m_stream.async_handshake(m_host, m_target, );




					//spawn a read
					co_spawn(m_stream.get_executor(), do_read(), [&](std::exception_ptr ptr) {
						if (ptr) {
							std::rethrow_exception(ptr); //rethrow or handle??
						}
					});
				}
				//do functions
				/**
				* Works as a client 
				*/
				awaitable<void> do_write() {
					std::error_code ec;
					size_t size = 0;
					size_t retry = 0;
					while(!m_msg_que.empty()) {
						
						auto& msg = m_msg_que.front();

						auto& header = std::get<0>(msg);
						auto& body = std::get<1>(msg);

						//crc?

						const std::array<boost::asio::const_buffer, 2> bufs{ boost::asio::buffer(std::addressof(header),
							wb_message_header_length), boost::asio::buffer(body) };
						timer_t timer(co_await boost::asio::this_coro::executor);
						timer.expires_after(1s); //write for one second
						auto complete = co_await (m_stream.async_write(bufs) || timer.async_wait());
						switch (complete.index())
						{
						case 0:
							timer.cancel();
							retry = 0;
							break;
						case 1:
						{
							//time out, retry? 
							if (retry < 3) {
								retry++;
								continue;
							}
							std::tie(ec) = std::get<1>(complete);
							pof::base::task_manager::instance().service().post([&, msg = std::move(msg)]() {
								recieve_sig(ec, msg);
							});
							m_msg_que.pop();
						}
						default:
							break;
						}
						if (ec) {
							fail(ec);
							co_return;
						}
						m_msg_que.pop();
					}
				}
				/*read is still a problem*/
				awaitable<void> do_read() {
					for (;;) {
						//read header
						std::error_code ec;
						size_t size = 0;
						std::array<std::uint8_t, wb_message_header_length> buf = {};
						auto header = new (buf.data()) wb_message_header;
						wb_message_body body;

						timer_t timer(co_await boost::asio::this_coro::executor);
						timer.expires_after(1s);

						auto complete = co_await (m_stream.async_read_some(m_read_buf.prepare(wb_message_header_length))
								|| timer.async_wait());
						switch (complete.index())
						{
						case 0:
							timer.cancel();
							break;
						case 1:
							//what happens if we timeout ? 
							continue; //
						default:
							break;
						}

						std::tie(ec, size) = std::get<0>(complete);
						if (ec ||  size != wb_message_header_length) {
							fail(ec); //failing here throws an execption, should it? 
						}
						m_read_buf.commit(wb_message_header_length);
						auto mutbuf = m_read_buf.data();
						std::copy(static_cast<std::uint8_t*>(mutbuf.data()), 
								static_cast<std::uint8_t*>(mutbuf.data()) + mutbuf.size(), buf.begin());
						m_read_buf.consume(wb_message_header_length);

						if (header->mMessageLength != 0) {
							//read body
							timer.expires_after(5s); //reading body data might take longer
							body.resize(header->mMessageLength);
							auto complete2 = co_await (m_stream.async_read_some(m_read_buf.prepare(header->mMessageLength)) || 
									timer.async_wait());
							switch (complete2.index())
							{
							case 0:
								timer.cancel();
								break;
							case 1:
								// time out retry or fail?
								continue; //ignore this data block and try to read another data block from header ? 
							default:
								break;
							}
							//carry on reading
							std::tie(ec, size) = std::get<0>(complete2);
							if (ec || size != header->mMessageLength) {
								fail(ec);
								co_return;
							}
							m_read_buf.commit(header->mMessageLength);
							mutbuf = m_read_buf.data();
							std::copy(static_cast<std::uint8_t*>(mutbuf.data()), 
							static_cast<std::uint8_t*>(mutbuf.data()) + mutbuf.size(), body.begin());
							m_read_buf.consume(header->mMessageLength);

						}

						wb_message msg{ *header, body };
						pof::base::task_manager::instance().service().post([&, msg = std::move(msg)]() {
							recieve_sig(std::error_code{}, msg);
						});
					}
				}


				void fail(std::error_code ec) {
					const size_t code = ec.value();
					if (code == boost::beast::errc::connection_aborted ||
							code == boost::asio::error::operation_aborted ||
							boost::system::error_code(ec) == boost::beast::websocket::error::closed) {
						return;
					}
					throw std::system_error(ec);
				}

				void control_frame(boost::beast::websocket::frame_type frame, boost::beast::string_view payload) {
					//timers??
				}

				recieve_signal_t recieve_sig;
			private:
				tcp::resolver m_resolver;

				std::string m_host;
				std::string m_target;

				boost::lockfree::spsc_queue<wb_message> m_msg_que{100}; //only buffer a hundrend messages
				boost::beast::flat_buffer m_read_buf;
				boost::beast::websocket::stream<beast::ssl_stream<tcp_stream>> m_stream;
			};
		}
		

		

		
	}
}

