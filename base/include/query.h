#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <exception>
#include <future>
#include <tuple>
#include <type_traits>

#include <boost/signals2.hpp>
#include <boost/mysql.hpp>
#include <boost/noncopyable.hpp>

#include <spdlog/spdlog.h>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/noncopyable.hpp>

#include "Data.h"
#include "errc.h"
#include "relation.h"

//represents data and query read or written to the database
using namespace boost::asio::experimental::awaitable_operators;
namespace pof {
	namespace base {

		boost::mysql::datetime to_mysql_datetime(const pof::base::data::datetime_t& tt);
		boost::mysql::blob to_mysql_uuid(const pof::base::data::duuid_t& duuid);

		template<typename manager>
		struct query : public std::enable_shared_from_this<query<manager>>, private boost::noncopyable {
			using default_token = boost::asio::as_tuple_t<boost::asio::use_awaitable_t<>>;
			using timer_t = default_token::as_default_on_t<boost::asio::steady_timer>;

			using shared_t = std::enable_shared_from_this<query<manager>>;
			static constexpr auto tuple_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);

			std::string m_sql;
			std::shared_ptr<pof::base::data> m_data;

			boost::mysql::diagnostics m_diag; //server diagonis
			std::shared_ptr<manager> m_manager; //must be the database manager
			std::promise<std::shared_ptr<pof::base::data>> m_promise;

			manager::conn_ptr m_connection;
			std::optional<timer_t> m_waittime;
			bool m_hold_connection = false;


			query(std::shared_ptr<manager> man = nullptr, const std::string& sql = ""s) : m_manager(man), m_sql(sql) {
				m_data = std::make_shared<pof::base::data>();
			}


			void unborrow() {
				if (m_connection) {
					m_manager->unborrow(m_connection);
					m_connection.reset();
				}
			}

			virtual ~query() {}

			std::future<std::shared_ptr<pof::base::data>> get_future(){
				return m_promise.get_future();
			}
			//Text query
			virtual boost::asio::awaitable<void> operator()() {
				auto this_ = shared_t::shared_from_this(); //hold till we leave the coroutine
				
				boost::mysql::results result;

				timer_t timer(co_await boost::asio::this_coro::executor);
				timer.expires_after(std::chrono::seconds(60));
				try {
					std::error_code ec;
					auto complete
						= co_await(m_connection->async_query(m_sql, result, tuple_awaitable)
							|| timer.async_wait());

					switch (complete.index())
					{
					case 0:
						timer.cancel();
						ec = std::get<0>(std::get<0>(complete));
						boost::mysql::throw_on_error(ec, m_diag);
						break;
					case 1:
						//what happens if we timeout ?
						//signal the query on timeout ...
						ec = boost::system::error_code(boost::asio::error::timed_out);
						boost::mysql::throw_on_error(ec, m_diag);
						break; //
					default:
						break;
					}

					if (!result.has_value()) {
						//query did not return a value, is this an error ?
						m_promise.set_value(nullptr); //set an empty data ? 
						if(m_waittime.has_value()) m_waittime->cancel();
						co_return;
					}
					else if (result.rows().empty()) {
						m_promise.set_value(m_data); //set an empty data ?
						if(m_waittime.has_value()) m_waittime->cancel();
						co_return;
					}
					else {
						const auto& meta = result.meta();
						auto& datameta = m_data->get_metadata();
						datameta.reserve(meta.size());
						for (const auto& m : meta) {
							auto k = m.type();
							switch (k)
							{
							case boost::mysql::column_type::bigint:
								datameta.emplace_back(pof::base::data::kind::uint64);
								break;
							case boost::mysql::column_type::smallint:
								datameta.emplace_back(pof::base::data::kind::uint32);
								break;
							case boost::mysql::column_type::text:
								datameta.emplace_back(pof::base::data::kind::text);
								break;
							case boost::mysql::column_type::json:
								datameta.emplace_back(pof::base::data::kind::text);
								break;
							case boost::mysql::column_type::double_:
								datameta.emplace_back(pof::base::data::kind::float64);
								break;
							case boost::mysql::column_type::float_:
								datameta.emplace_back(pof::base::data::kind::float32);
								break;
							case boost::mysql::column_type::datetime:
								datameta.emplace_back(pof::base::data::kind::datetime);
								break;
							case boost::mysql::column_type::blob:
								datameta.emplace_back(pof::base::data::kind::float32);
								break;
							case boost::mysql::column_type::int_:
								datameta.push_back(pof::base::data::kind::int32);
								break;
							case boost::mysql::column_type::unknown:
								datameta.push_back(pof::base::data::kind::null);
								break;
							default:
								break;
							}
						}
						const auto& rows = result.rows();

						m_data->reserve(rows.size());
						for (const auto& row : rows) {
							//copy data
							size_t i = 0;
							std::vector<pof::base::data::data_t> v;
							v.resize(meta.size()); //

							for (const auto& m : meta)
							{
								auto k = m.type();
								
								switch (k)
								{
								case boost::mysql::column_type::int_:
								case boost::mysql::column_type::bigint:
								case boost::mysql::column_type::decimal:
									v[i] = row.at(i).as_int64();
									break;
								case boost::mysql::column_type::float_:
									v[i] = row.at(i).as_float();
									break;
								case boost::mysql::column_type::double_:
									v[i] = row.at(i).as_double();
									break;
								
								case boost::mysql::column_type::bit:
								case boost::mysql::column_type::varbinary:
								case boost::mysql::column_type::blob:
								{
									auto bv = row.at(i).as_blob();
									pof::base::data::blob_t blob;
									blob.reserve(bv.size());

									std::ranges::copy(bv, std::back_inserter(blob));
									v[i] = std::move(blob);
								}
									break;

								case boost::mysql::column_type::time:
								case boost::mysql::column_type::date:
								case boost::mysql::column_type::datetime:
								case boost::mysql::column_type::timestamp:
									v[i] = pof::base::data::datetime_t(row.at(i).as_datetime().as_time_point().time_since_epoch()); // wrong type
									break;
								case boost::mysql::column_type::binary:
								{
									//is char converted to blob or stirng
									auto collect = row.at(i).as_blob();
									const size_t size = collect.size();
									switch (size)
									{
										//reserving 16 bytes fixed length for uuids
									case 16:
									{
										boost::uuids::uuid uuid{};
										std::copy(collect.begin(), collect.end(), uuid.begin());
										v[i] = uuid;
										datameta[i] = pof::base::data::kind::uuid;
									}
									break;
									//reserving 17 bytes fixed length for currency
									case 17:
									{
										pof::base::currency cur;
										std::ranges::copy(collect, cur.data().begin());
										v[i] = cur;
										datameta[i] = pof::base::data::kind::currency;
									}
									break;
									default:
										break;
									}
								}
								break;
								case boost::mysql::column_type::varchar:
								case boost::mysql::column_type::text:
								case boost::mysql::column_type::enum_:
								case boost::mysql::column_type::set:
								case boost::mysql::column_type::json:
									v[i] = pof::base::data::text_t(row.at(i).as_string());
									break;
								case boost::mysql::column_type::unknown:
									break;
								default:
									break;
								}
								i++; //next column
							}
							m_data->emplace(std::move(v));

						}
						m_promise.set_value(m_data); //set the data
					}
				}catch (...) {
					m_promise.set_exception(std::current_exception());
			    }
				if(m_waittime.has_value()) m_waittime->cancel();
				co_return;
			}
		};

		//Statement queries may require arguments
		template<typename manager>
		struct querystmt : public query<manager> {
			using base_t = query<manager>;
			using row_t = std::vector<boost::mysql::field>;
			using data_t = std::vector<row_t>;
			boost::mysql::statement stmt;
			data_t m_arguments;

			querystmt(std::shared_ptr<manager> manager = nullptr) : base_t(manager) {}
			querystmt(std::shared_ptr<manager> manager, const std::string& sql) : base_t(manager){
				base_t::m_sql = sql;
			}

			boost::asio::awaitable<boost::system::error_code> close() {
				boost::system::error_code ec;
				if (stmt.valid()) {
					std::tie(ec) =  co_await base_t::m_connection->async_close_statement(stmt, base_t::tuple_awaitable);
				}
				co_return ec;
			}

			virtual ~querystmt() {
				if(stmt.valid()) base_t::m_connection->close_statement(stmt); //blocks might be a bottle neck
			}

			virtual boost::asio::awaitable<void> operator()() override {
				auto this_ = base_t::shared_t::shared_from_this(); //hold till we leave the coroutine

				std::error_code ec;
				auto& conn = base_t::m_connection;
				try {
					typename base_t::timer_t timer(co_await boost::asio::this_coro::executor);
					timer.expires_after(std::chrono::minutes(1));

					if (!stmt.valid()) {
						auto complete = co_await(conn->async_prepare_statement(base_t::m_sql, base_t::m_diag, base_t::tuple_awaitable) || timer.async_wait());
						switch (complete.index())
						{
						case 0:
							timer.cancel();
							ec = std::get<0>(std::get<0>(complete));
							stmt = std::get<1>(std::get<0>(complete));
							break;
						case 1:
							//what happens if we timeout ?
							//signal the query on timeout ...
							ec = boost::system::error_code(boost::asio::error::timed_out);
							boost::mysql::throw_on_error(ec, base_t::m_diag);
							break;
						default:
							break;
						}
					}

					boost::mysql::throw_on_error(ec, base_t::m_diag);
					if (m_arguments.empty()) {
						//use a text query if no arguments are needed
						ec = boost::system::error_code(std::make_error_code(pof::base::errc::no_arguments));
						boost::mysql::throw_on_error(ec, base_t::m_diag);
					}
					for (auto& arg : m_arguments) {
						boost::mysql::results result;
						timer.expires_after(std::chrono::minutes(1));
						auto comp = co_await(conn->async_execute(stmt.bind(arg.begin(), arg.end()), result, base_t::m_diag, base_t::tuple_awaitable) ||
							timer.async_wait());
						switch (comp.index())
						{
						case 0:
							timer.cancel();
							ec = std::get<0>(std::get<0>(comp));
							break;
						case 1:
							//what happens if we timeout ?
							//signal the query on timeout ...
							ec = boost::system::error_code(boost::asio::error::timed_out);
							boost::mysql::throw_on_error(ec, base_t::m_diag);
						default:
							break;
						}

						boost::mysql::throw_on_error(ec, base_t::m_diag);
						if (!result.has_value()) {
							base_t::m_promise.set_value(nullptr);
							if(base_t::m_waittime.has_value()) base_t::m_waittime->cancel();
							co_return;
						}
						else if (result.empty()) {
							base_t::m_promise.set_value(base_t::m_data);
							if (base_t::m_waittime.has_value()) base_t::m_waittime->cancel();
							co_return;
						}
						else {
							const auto& meta = result.meta();
							auto& datameta = base_t::m_data->get_metadata();
							if (datameta.empty()) {
								datameta.reserve(meta.size());
								for (const auto& m : meta) {
									auto k = m.type();
									switch (k)
									{
									case boost::mysql::column_type::bigint:
										datameta.emplace_back(pof::base::data::kind::uint64);
										break;
									case boost::mysql::column_type::smallint:
										datameta.emplace_back(pof::base::data::kind::uint32);
										break;
									case boost::mysql::column_type::text:
										datameta.emplace_back(pof::base::data::kind::text);
										break;
									case boost::mysql::column_type::json:
										datameta.emplace_back(pof::base::data::kind::text);
										break;
									case boost::mysql::column_type::double_:
										datameta.emplace_back(pof::base::data::kind::float64);
										break;
									case boost::mysql::column_type::float_:
										datameta.emplace_back(pof::base::data::kind::float32);
										break;
									case boost::mysql::column_type::date:
									case boost::mysql::column_type::time:
									case boost::mysql::column_type::timestamp:
									case boost::mysql::column_type::datetime:
										datameta.emplace_back(pof::base::data::kind::datetime);
										break;
									case boost::mysql::column_type::char_:

										break;
									case boost::mysql::column_type::bit:
									case boost::mysql::column_type::varbinary:
									case boost::mysql::column_type::blob:
										datameta.emplace_back(pof::base::data::kind::blob);
										break;
									case boost::mysql::column_type::int_:
										datameta.push_back(pof::base::data::kind::int32);
										break;
									case boost::mysql::column_type::unknown:
										datameta.push_back(pof::base::data::kind::null);
										break;
									default:
										break;
									}
								}
							}

							const auto& rows = result.rows();
							if(base_t::m_data->empty())
								base_t::m_data->reserve(rows.size());
							for (const auto& row : rows) {
								//copy data
								size_t i = 0;
								std::vector<pof::base::data::data_t> v;
								v.resize(meta.size()); //

								for (const auto& m : meta)
								{
									auto k = m.type();

									switch (k)
									{
									case boost::mysql::column_type::tinyint:
										v[i] = static_cast<std::uint8_t>(row.at(i).as_int64());
										break;
									case boost::mysql::column_type::int_:
										v[i] = row.at(i).as_int64();
										break;
									case boost::mysql::column_type::bigint:
									case boost::mysql::column_type::decimal:
										v[i] = row.at(i).as_uint64();
										break;
									case boost::mysql::column_type::float_:
										v[i] = row.at(i).as_float();
										break;
									case boost::mysql::column_type::double_:
										v[i] = row.at(i).as_double();
										break;

									case boost::mysql::column_type::bit:
									case boost::mysql::column_type::varbinary:
									case boost::mysql::column_type::blob:
									{
										auto bv = row.at(i).as_blob();
										pof::base::data::blob_t blob;
										blob.reserve(bv.size());

										std::ranges::copy(bv, std::back_inserter(blob));
										v[i] = std::move(blob);
									}
									break;

									case boost::mysql::column_type::date:
									case boost::mysql::column_type::time:
									case boost::mysql::column_type::datetime:
									case boost::mysql::column_type::timestamp:
										v[i] = pof::base::data::datetime_t(std::chrono::time_point_cast<
											pof::base::data::clock_t::duration>(row.at(i).as_datetime().as_time_point())); // wrong type
										break;
									case boost::mysql::column_type::binary:
									{
										auto collect = row.at(i).as_blob();
										const size_t size = collect.size();
										switch (size)
										{
										//reserving 16 bytes fixed length for uuids
										case 16:
										{
											boost::uuids::uuid uuid{};
											std::copy(collect.begin(), collect.end(), uuid.begin());
											v[i] = uuid;
											datameta[i] = pof::base::data::kind::uuid;
										}
											break;
										//reserving 17 bytes fixed length for currency
										case 17:
										{
											pof::base::currency cur;
											std::ranges::copy(collect, cur.data().begin());
											v[i] = cur;
											datameta[i] = pof::base::data::kind::currency;
										}
											break;
										default:
											break;
										}
									}
									break;
									case boost::mysql::column_type::varchar:
									case boost::mysql::column_type::text:
									//case boost::mysql::column_type::enum_:
									//case boost::mysql::column_type::set:
									case boost::mysql::column_type::json:
										v[i] = pof::base::data::text_t(row.at(i).as_string());
										break;
									case boost::mysql::column_type::unknown:
										break;
									default:
										break;
									}
									i++; //next column
								}
								base_t::m_data->emplace(std::move(v));
							}
						}
						base_t::m_promise.set_value(base_t::m_data); //data moved into the datamodels cache
					}
				} catch (...) {
					base_t::m_promise.set_exception(std::current_exception());
				}
				if (base_t::m_waittime.has_value()) base_t::m_waittime->cancel();
				co_return;
			}
		};
	}
};