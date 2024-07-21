#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>
#include <thread>
#include <boost/asio/executor_work_guard.hpp>
#include <memory>
#include <mutex>

namespace pof
{
	namespace base {
		class task_manager : private boost::noncopyable {
		public:
			task_manager(task_manager&& rhs) = delete;
			task_manager& operator=(task_manager&&) = delete;

			bool stop();
			constexpr inline boost::asio::io_service& service() { return m_service; }
			static task_manager& instance();
			task_manager();

		private:
			std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_service::executor_type>> m_workgaurd;
			boost::asio::io_service m_service;
			std::thread m_taskThread;
		};
	};
};