#pragma once
#pragma once
#ifdef  WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif //  WIN32

#include <boost/mysql.hpp>
#include <boost/noncopyable.hpp>



#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/as_tuple.hpp>


#include <spdlog/spdlog.h>
#include <memory>
#include <deque>
#include <shared_mutex>
#include <chrono>
#include <atomic>
#include <set>
#include <mutex>
#include <condition_variable>

#include "query.h"
#include "errc.h"




using namespace boost::asio::experimental::awaitable_operators;
using namespace std::literals::chrono_literals;
constexpr auto tuple_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);
namespace pof {
	namespace base {
		class databasemysql : private boost::noncopyable, public std::enable_shared_from_this<databasemysql>
		{
		public:
			constexpr static int connection_max = 10;
			using connection_t = boost::mysql::tcp_ssl_connection;
			using conn_ptr = std::shared_ptr<connection_t>;

			databasemysql(boost::asio::io_context& ios);
			bool create_pool();

			boost::asio::awaitable<std::error_code> connect(conn_ptr conn, std::string hostname, 
			std::string port,
			std::string user, std::string pwd);

			bool connect();
			//Adds a query to the queue
			bool push(std::shared_ptr<pof::base::query<databasemysql>> query);
			boost::asio::awaitable<bool> retry(std::shared_ptr<pof::base::query<databasemysql>> query);

			void setupssl();
			boost::asio::awaitable<void> runquery(std::shared_ptr<pof::base::query<databasemysql>> query);


			//also closes 
			void disconnect();
			bool m_isconnected = false;

			conn_ptr borrow();
			void unborrow(conn_ptr conn);

			void set_params(const std::string& host, const std::string& sport, const std::string& suser, const std::string& spwd);
			void use_database(const std::string& name);
			void create_database(const std::string& name);
		private:
			std::string hostname;
			std::string port;
			std::string user;
			std::string pwd;

			conn_ptr create();


			//ref to the operating system fasilities
			boost::asio::io_context& mIos;
			boost::asio::ssl::context mSsl;

			boost::asio::ip::tcp::resolver m_resolver;

			std::atomic<bool> m_isrunning;
			std::condition_variable mStmtConditionVarible;

			

			//connection pool
			std::mutex m_mutex;
			std::set<conn_ptr> m_borrowed;
			std::list<conn_ptr> m_pool;
		};

		using dataquerybase = pof::base::query<databasemysql>;
		using datastmtquery = pof::base::querystmt<databasemysql>;
	}
};