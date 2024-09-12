#include "taskmanager.h"

pof::base::task_manager::task_manager()
{
	size_t thread_count = std::thread::hardware_concurrency() >> 2;
	m_service = std::make_unique<boost::asio::thread_pool>(thread_count);
}

