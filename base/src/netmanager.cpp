#pragma once
#include <netmanager.h>


pof::base::net_manager::net_manager()
	: m_ssl{boost::asio::ssl::context_base::sslv23_client}{
	auto ec = setupssl();
	if (ec) {
		
	}
	m_workgaurd = std::make_unique<net::executor_work_guard<net::io_context::executor_type>>(m_io.get_executor());
	m_thread = std::move(std::thread{static_cast<size_t(net::io_context::*)()>(&net::io_context::run), std::ref(m_io)});
}

bool pof::base::net_manager::stop()
{
	m_workgaurd.reset(nullptr);
	m_io.stop();
	m_thread.join();
	return true;
}

std::error_code pof::base::net_manager::setupssl()
{
	//this would change
	m_ssl.set_verify_mode(net::ssl::verify_none);
	return std::error_code();
}
