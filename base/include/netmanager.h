#pragma once
#include "net.h"
#include "errc.h"

#include <boost/noncopyable.hpp>

#include <thread>
#include <mutex>


namespace pof
{
	namespace base {
		class net_manager : public boost::noncopyable
		{
		public:
			net_manager();
			bool stop();
			std::error_code setupssl();

			inline net::io_context& io() { return m_io; }
			inline net::ssl::context& ssl() { return m_ssl; }

		private:
			std::unique_ptr<net::executor_work_guard<boost::asio::io_context::executor_type>> m_workgaurd;
			net::io_context m_io;
			net::ssl::context m_ssl;
			std::thread m_thread;

		};
	};
};