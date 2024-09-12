#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/noncopyable.hpp>
#include <thread>
#include <memory>


namespace pof
{
	namespace base {
		class task_manager : private boost::noncopyable {
		public:
			task_manager(task_manager&& rhs) = delete;
			task_manager& operator=(task_manager&&) = delete;

			inline boost::asio::thread_pool& tp() { return *m_service; }
			task_manager();

		private:
			std::unique_ptr<boost::asio::thread_pool> m_service;
		};
	};
};