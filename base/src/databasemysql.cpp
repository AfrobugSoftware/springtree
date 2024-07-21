#include "databasemysql.h"


boost::mysql::datetime pof::base::to_mysql_datetime(const pof::base::data::datetime_t& tt) {
	auto dt = std::chrono::time_point_cast<boost::mysql::datetime::time_point::duration,
		pof::base::data::clock_t, std::chrono::system_clock::duration>(tt);
	return boost::mysql::datetime(dt);
}

boost::mysql::blob pof::base::to_mysql_uuid(const pof::base::data::duuid_t& duuid) {
	return boost::mysql::blob(duuid.begin(), duuid.end());
}


pof::base::databasemysql::databasemysql(boost::asio::io_context& ios)
	: m_resolver(boost::asio::make_strand(ios.get_executor())), mIos(ios), mSsl{boost::asio::ssl::context_base::sslv23_client} {
}

bool pof::base::databasemysql::create_pool()
{
	for (int i = 0; i < connection_max; i++) {
		auto sp = std::make_shared<boost::mysql::tcp_ssl_connection>(boost::asio::make_strand(mIos), mSsl);
		m_pool.push_back(sp);
	}
	return true;
}

boost::asio::awaitable<std::error_code> pof::base::databasemysql::connect(conn_ptr conn,std::string hostname, std::string port, std::string user, std::string pwd)
{
	if (hostname.empty() || port.empty() || user.empty() || pwd.empty())
			throw std::system_error(std::make_error_code(pof::base::errc::no_database_hostname));

	auto&& [ec1, endpoints] = co_await m_resolver.async_resolve(hostname, port, tuple_awaitable);
	if (ec1) 
		throw std::system_error(ec1);
	
	boost::mysql::handshake_params params{ user, pwd };
	auto [ec] = co_await conn->async_connect(*endpoints.begin(), params, tuple_awaitable);
	if (ec)
		throw std::system_error(ec);
	else {
		m_pool.push_back(conn);
	}
	co_return std::error_code{};
}

bool pof::base::databasemysql::connect()
{
	auto results = m_resolver.resolve(hostname, port);
	if (results.empty() || m_pool.empty()) return false;

	boost::mysql::handshake_params params{ user, pwd };
	boost::mysql::error_code ec;
	boost::mysql::diagnostics d;
	m_isconnected = false;
	auto iter = std::remove_if(m_pool.begin(), m_pool.end(), [&](auto& conn) {
		conn->connect(*results.begin(), params, ec, d);
		spdlog::info(ec.message());
		return static_cast<bool>(ec);
	});
	m_pool.erase(iter, m_pool.end());

	if (!m_pool.empty()) m_isconnected = true;
	return m_isconnected;
}

bool pof::base::databasemysql::push(std::shared_ptr<pof::base::query<databasemysql>> query)
{
	try {
		if (query->m_hold_connection && query->m_connection){
			boost::asio::co_spawn(query->m_connection->get_executor(), runquery(query), boost::asio::detached);
		}
		else {
			auto sp = borrow(); //get a connection
			if (sp) {
				query->m_connection = sp;
				boost::asio::co_spawn(sp->get_executor(), runquery(query), boost::asio::detached);
			}
			else {
				//no connection
				//caancel the timer
				if (query->m_waittime.has_value()) query->m_waittime->cancel();
				spdlog::error("No connection avaliable");
				return false;
			}
		}
	}
	catch (const std::system_error& err) {
		//what do we do here
		auto& ec = err.code();
		if (ec == std::make_error_code(pof::base::errc::no_connection_avaliable)) {
			//no connection avalibale, wait for connection
			return false;
		}
	}
	return true;
}

boost::asio::awaitable<bool>
	pof::base::databasemysql::retry(std::shared_ptr<pof::base::query<databasemysql>> query)
{
	int retry_count = 3;
	try {
		pof::base::dataquerybase::timer_t timer(co_await boost::asio::this_coro::executor);
		timer.expires_after(10ms);
		auto&& [ec] = co_await timer.async_wait();
		if (!ec) {
			while (retry_count > 0) {
				bool pushed = push(query->shared_from_this());
				if (pushed) co_return true;

				timer.expires_after(10ms);
				std::tie(ec) = co_await timer.async_wait();
				retry_count--;
			}
		}
		co_return false;
	}
	catch (const std::exception& exp) {
		co_return false;
	}
}

void pof::base::databasemysql::setupssl()
{
}

boost::asio::awaitable<void> pof::base::databasemysql::runquery(std::shared_ptr<pof::base::query<databasemysql>> query)
{
	if (m_isconnected) {
		try {
			//execute the query
			co_await(*query)();
			if (!query->m_hold_connection)
			{
				unborrow(query->m_connection);
				query->m_connection.reset();
			}
		}
		catch (...) {
			std::rethrow_exception(std::current_exception());
		}
	}
}


void pof::base::databasemysql::disconnect()
{
	//synchronous block until all connection block
	std::unique_lock<std::mutex> lock(m_mutex);
	for (auto& conn : m_pool) {
		conn->quit();
	}
	m_pool.clear();
	for (auto& conn : m_borrowed) {
		conn->quit();
	}
	m_borrowed.clear();
}

pof::base::databasemysql::conn_ptr pof::base::databasemysql::borrow()
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	//no free connections
	if (m_pool.empty()) {
		//are theere any crashed connections listed as borrowed
		if (!m_borrowed.empty()) {
			for (auto& sp : m_borrowed) {
				if (sp.use_count() <= 1) {
					//this connection has been abandoned 
					try {
						m_borrowed.erase(sp);
						m_pool.push_back(sp);
					}
					catch (std::exception& ec) {
						throw std::system_error(std::make_error_code(pof::base::errc::no_connection_avaliable));
					}
				}
			}
		}
		
		//non found
		if (m_pool.empty()) {
			return nullptr;
		}
	}

	auto sp = m_pool.front();
	m_pool.pop_front();
	m_borrowed.insert(sp);
	return sp;
}

void pof::base::databasemysql::unborrow(conn_ptr conn)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_pool.push_back(conn);
	m_borrowed.erase(conn);
}

void pof::base::databasemysql::set_params(const std::string& host, const std::string& sport, const std::string& suser, const std::string& spwd)
{
	hostname = host;
	port = sport;
	user = suser;
	pwd = spwd;
}

void pof::base::databasemysql::use_database(const std::string& name)
{
	auto sql = std::format("USE {};", name);
	boost::mysql::results result;
	for (auto iter = m_pool.begin(); iter != m_pool.end(); iter++) {
		(*iter)->query(sql, result);
		if (!result.has_value()) {
			(*iter)->close();
			m_pool.erase(iter);
		}
	}
}

void pof::base::databasemysql::create_database(const std::string& name)
{
	try {
		auto query = std::make_shared<pof::base::query<pof::base::databasemysql>>(shared_from_this());
		query->m_sql = fmt::format("CREATE DATABASE IF NOT EXISTS {};", name);
		auto fut = query->get_future();
		bool pushed = push(query);

		if(pushed) (void)fut.get();
	}
	catch (const std::exception& exp) {
		spdlog::error("Failed to create database");
	}

}

pof::base::databasemysql::conn_ptr pof::base::databasemysql::create()
{
	return conn_ptr();
}
