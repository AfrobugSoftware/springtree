#include "taskmanager.h"
static std::once_flag g_flag;
static std::unique_ptr<pof::base::task_manager> g_task_manager;


bool pof::base::task_manager::stop()
{
	m_workgaurd.reset(nullptr);
	m_service.stop();
	if (m_taskThread.joinable()) m_taskThread.join();
	return true;
}

pof::base::task_manager& pof::base::task_manager::instance()
{
	std::call_once(g_flag, [&]() {
		if (!g_task_manager) {
			g_task_manager = std::make_unique<pof::base::task_manager>();
		}
	});
	return *g_task_manager;
}

pof::base::task_manager::task_manager()
{
	m_workgaurd = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_service::executor_type>>(m_service.get_executor());
	m_taskThread = std::move(std::thread{ static_cast<size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run), std::ref(m_service) });
}

