#pragma once
#include "net.h"
#include "packages.h"
#include <future>
#include <memory>
#include <type_traits>
#include <boost/noncopyable.hpp>

namespace pof {
	namespace base {

		//sends and recieves packages
		//post for sending a package and get for recieving a package
		template<boost::beast::http::verb verb, typename req_body
				= std::conditional_t<verb == http::verb::post, http::vector_body<pack_t::value_type>, http::empty_body>>
		class datareq : public std::enable_shared_from_this<datareq<verb, req_body>>, private boost::noncopyable {
		public:
			using session_t = pof::base::ssl::session< std::conditional_t<verb == http::verb::post,
			http::string_body, http::vector_body<pack_t::value_type>> ,req_body>;
			using future_t = typename session_t::future_t;
			using clock_t = std::chrono::system_clock;

			datareq(const std::string hst, const std::string& tar, const std::string& port):
				m_host(hst),
				m_port(port),
				m_target(tar) {}

			datareq(datareq&& dr) noexcept : m_host(std::move(dr.m_host)),
				m_port(std::move(dr.m_port)),
				m_target(std::move(dr.m_target)),
				req_future(std::move(dr.req_future)) {}

			datareq& operator=(datareq&& dr) noexcept {
				m_host = std::move(dr.m_host);
				m_port = std::move(dr.m_port);
				m_target = std::move(dr.m_target);
				req_future = std::move(dr.req_future);
			}



			//blocks 
			void operator()() const {
				auto req = std::make_shared<session_t>();
				req_future = std::move(req->req<verb>(m_host, m_target, m_port));
			}
			void operator()(const pack_t& pack) {
				if constexpr (verb != http::verb::post) return;
				auto req = std::make_shared<session_t>();
				req_future = std::move(req->req<verb>(m_host, m_target, m_port, std::move(pack)));
			}

			inline std::future_status wait_for(clock_t::duration dur) {
				return req_future.wait_for(dur);
			}
			const std::string& get_taget() const { return m_target; }
			inline void set_target(const std::string& tar) { m_target = tar; }
		private:
			std::string m_host;
			std::string m_port;
			std::string m_target;
			future_t req_future;

		};

	};
};
