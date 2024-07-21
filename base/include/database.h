#pragma once
#include "relation.h"
#include <sqlite3.h>
#include <boost/noncopyable.hpp>

#include <unordered_map>
#include <optional>
#include <filesystem>
#include <exception>
#include <utility>
#include <functional>
#include <date/date.h>
namespace fs = std::filesystem;
namespace pof {
	namespace base {
		//for SQL with ? instead of a parameter
		namespace detail {
			template<typename T>
			inline bool do_bind(sqlite3_stmt* statement, const T& value, size_t position)
			{
				using arg_type = std::decay_t<T>;
				static_assert(is_database_type<arg_type>::value, "Tuple type is not a valid database type");
				if constexpr (std::is_integral_v<arg_type>)
				{
					//64-bit int
					if constexpr (sizeof(arg_type) == sizeof(std::uint64_t))
					{
						return (SQLITE_OK == sqlite3_bind_int64(statement, position, value));
					}
					return (SQLITE_OK == sqlite3_bind_int(statement, position, value));
				}
				else if constexpr (std::is_floating_point_v<arg_type>)
				{
					if constexpr (std::is_same_v<arg_type, float>)
					{
						return (SQLITE_OK == sqlite3_bind_double(statement, position, static_cast<double>(value)));
					}
					return (SQLITE_OK == sqlite3_bind_double(statement, position, value));
				}
				//need to add support for other string formats
				else if constexpr (std::is_same_v<arg_type, pof::base::data::text_t>)
				{
					if (!value.empty())
					{
						return (SQLITE_OK == sqlite3_bind_text(statement, position, value.c_str(), value.size(), SQLITE_TRANSIENT));
					}
					else {
						return (SQLITE_OK == sqlite3_bind_null(statement, position));
					}
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::blob_t>)
				{
					if (value.empty())
					{
						//write null??? or just leave it so that, we would just bind null on empty vector
						return (SQLITE_OK == sqlite3_bind_null(statement, position));
					}
					return (SQLITE_OK == sqlite3_bind_blob(statement, position, value.data(), value.size(), SQLITE_TRANSIENT));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::datetime_t>)
				{
					auto rep = value.time_since_epoch().count();
					return (SQLITE_OK == sqlite3_bind_int64(statement, position, rep));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::duuid_t>)
				{
					return (SQLITE_OK == sqlite3_bind_blob(statement, position, value.data, value.size(), SQLITE_TRANSIENT));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::currency_t>) {
					return (SQLITE_OK == sqlite3_bind_blob(statement, position, value.data().data(), value.data().size(), SQLITE_TRANSIENT));
				}
				else
				{
					return false;
				}

			}
			/**
			* Parameters must be specified in the SQL query as either ? or :name
			* any other format would lead to undefiend behaviour
			*/
			template<typename T>
			bool do_bind_para(sqlite3_stmt* statement, const T& value, std::string_view name)
			{
				using arg_type = std::decay_t<T>;
				static_assert(is_database_type<arg_type>::value, "Tuple type is not a valid database type");
				const std::string p_name = fmt::format(":{}", name);
				int position = sqlite3_bind_parameter_index(statement, p_name.c_str());
				if (position == 0) return false;

				if constexpr (std::is_integral_v<arg_type>)
				{
					//64-bit integers have speical functions to upload in sqlite
					if constexpr (sizeof(arg_type) == sizeof(std::uint64_t))
					{
						return (SQLITE_OK == sqlite3_bind_int64(statement, position, value));
					}
					return (SQLITE_OK == sqlite3_bind_int(statement, position, value));
				}
				else if constexpr (std::is_floating_point_v<arg_type>)
				{
					if constexpr (std::is_same_v<arg_type, float>)
					{
						return (SQLITE_OK == sqlite3_bind_double(statement, position, static_cast<double>(value)));
					}
					return (SQLITE_OK == sqlite3_bind_double(statement, position, value));
				}
				//need to add support for other string formats
				else if constexpr (std::is_same_v<arg_type, pof::base::data::text_t>)
				{
					if (!value.empty())
					{
						return (SQLITE_OK == sqlite3_bind_text(statement, position, value.c_str(), value.size(), SQLITE_TRANSIENT));
					}
					return (SQLITE_OK == sqlite3_bind_null(statement, position));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::blob_t>)
				{
					if (value.empty())
					{
						//write null??? or just leave it so that, we would just bind null on empty vector
						return (SQLITE_OK == sqlite3_bind_null(statement, position));
					}
					return (SQLITE_OK == sqlite3_bind_blob(statement, position, value.data(), value.size(), SQLITE_TRANSIENT));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::datetime_t>)
				{
					pof::base::data::clock_t::rep rep = value.time_since_epoch().count();
					return (SQLITE_OK == sqlite3_bind_int64(statement, position, rep));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::duuid_t>)
				{
					return (SQLITE_OK == sqlite3_bind_blob(statement, position, value.data, value.size(), SQLITE_TRANSIENT));
				}
				else if constexpr (std::is_same_v<arg_type, pof::base::data::currency_t>) {
					return (SQLITE_OK == sqlite3_bind_blob(statement, position, value.data().data(), value.data().size(), SQLITE_TRANSIENT));
				}
				else if constexpr (std::is_enum_v<arg_type>) {
					auto en = static_cast<std::uint32_t>(value);
					return (SQLITE_OK == sqlite3_bind_int(statement, position, en));
				}
				else
				{
					return false;
				}

			}

			template<typename arg_type, size_t N>
			auto do_retrive(sqlite3_stmt* statement) -> std::tuple<arg_type>
			{
				const size_t col = (size_t)sqlite3_column_count(statement) - (N + 1);
				using arg_t = typename std::decay_t<arg_type>;

				static_assert(is_database_type<arg_t>::value, "Type in tuple is not a valid database type");
				if constexpr (std::is_integral_v<arg_t>)
				{
					//64 bit intergers has a special download function in sqlite
					if constexpr (sizeof(arg_t) == sizeof(std::uint64_t))
					{
						return std::make_tuple(sqlite3_column_int64(statement, col));
					}
					return std::make_tuple(sqlite3_column_int(statement, col));
				}
				else if constexpr (std::is_floating_point_v<arg_t>)
				{
					if constexpr (std::is_same_v<arg_t, float>)
					{
						//sqlite uses REAL type as double, need to safely cast, removes annoying warnings
						return std::make_tuple(static_cast<arg_t>(sqlite3_column_double(statement, col)));
					}
					return std::make_tuple(sqlite3_column_double(statement, col));
				}
				else if constexpr (std::is_same_v<arg_t, pof::base::data::blob_t>)
				{
					const pof::base::data::blob_t::value_type* val_ptr = static_cast<const pof::base::data::blob_t::value_type*>(sqlite3_column_blob(statement, col));
					if (val_ptr)
					{
						const size_t size = sqlite3_column_bytes(statement, col);
						pof::base::data::blob_t vec(size);
						std::copy(val_ptr, val_ptr + size, vec.begin());
						return std::make_tuple(std::move(vec));
					}
					return std::make_tuple(pof::base::data::blob_t{});
				}
				else if constexpr (std::is_same_v<arg_t, pof::base::data::text_t>)
				{
					const char* txt = (const char*)(sqlite3_column_text(statement, col));
					if (txt)
					{
						return std::make_tuple(pof::base::data::text_t(txt));
					}
					return std::make_tuple(pof::base::data::text_t{});
				}
				else if constexpr (std::is_same_v<arg_t, pof::base::data::datetime_t>)
				{
					pof::base::data::clock_t::rep rep = sqlite3_column_int64(statement, col);
					return std::make_tuple(pof::base::data::datetime_t(pof::base::data::clock_t::duration(rep)));
				}
				else if constexpr (std::is_same_v<arg_t, pof::base::data::duuid_t>)
				{
					const pof::base::data::blob_t::value_type* val_ptr =
						static_cast<const pof::base::data::blob_t::value_type*>(sqlite3_column_blob(statement, col));
					if (val_ptr)
					{
						const size_t size = sqlite3_column_bytes(statement, col);
						if (size == 16) //128 bit ids
						{
							pof::base::data::duuid_t id;
							std::copy(val_ptr, val_ptr + size, id.data);
							return std::make_tuple(std::move(id));
						}
					}
					return std::make_tuple(boost::uuids::nil_uuid());
				}
				else if constexpr (std::is_same_v<arg_t, pof::base::data::currency_t>) {
					const pof::base::data::blob_t::value_type* val_ptr =
						static_cast<const pof::base::data::blob_t::value_type*>(sqlite3_column_blob(statement, col));
					if (val_ptr)
					{
						const size_t size = sqlite3_column_bytes(statement, col);
						pof::base::data::currency_t buf;
						std::copy(val_ptr, val_ptr + size, buf.data().data());
						return std::make_tuple(std::move(buf));

					}
					return std::make_tuple(pof::base::data::currency_t{});
				}
				else if constexpr (std::is_enum_v<arg_t>) {
					auto en = static_cast<arg_t>(sqlite3_column_int(statement, col));
					return std::make_tuple(en);
				}
			}

			template<size_t N>
			struct loop {
				template<typename tuple_t>
				static bool bind(sqlite3_stmt* stmt, const tuple_t& tuple) {
					bool ret = detail::loop<N - 1>::template bind(stmt, tuple);

					const auto& val = std::get<N>(tuple);
					bool ret2 = detail::do_bind(stmt, val, N + 1);
					return (ret2 && ret);
				}

				template<typename tuple_t, typename array_t>
				static bool bind_para(sqlite3_stmt* stmt, const tuple_t& tuple, array_t&& arr)
				{
					bool ret = loop<N - 1>::template bind_para(stmt, tuple, std::forward<array_t>(arr));
					const auto& val = std::get<N>(tuple);
					bool ret2 = detail::do_bind_para(stmt, val, arr[N]);
					return (ret2 && ret);
				}

				template<typename tuple_t>
				static auto retrive(sqlite3_stmt* statement)
				{
					constexpr size_t col = (std::tuple_size_v<tuple_t> - (N + 1));
					using arg_type = std::tuple_element_t<col, tuple_t>;
					auto t1 = do_retrive<arg_type, N>(statement);
					auto t2 = detail::loop<N - 1>::template retrive<tuple_t>(statement);

					return std::tuple_cat(std::move(t1), std::move(t2));
				}

				template<typename tuple_t>
				static bool build(tuple_t& tup, const pof::base::data::row_t& row)
				{
					auto& v = row.first;
					if (v.empty()) return false;
					using arg_type = std::tuple_element_t<N, tuple_t>;

					if (boost::variant2::holds_alternative<arg_type>(v[N])) {
						std::get<N>(tup) = boost::variant2::get<arg_type>(v[N]);
					}
					else {
						if constexpr (std::is_integral_v<arg_type>) {
							std::get<N>(tup) = static_cast<arg_type>(0);
						}
						else if constexpr (std::is_floating_point_v<arg_type>) {
							std::get<N>(tup) = static_cast<arg_type>(0.0f);
						}
						else {
							std::get<N>(tup) = arg_type{};
						}
					}
					return detail::loop<N - 1>::build(tup, row);
				}
			};

			template<>
			struct loop<0>
			{
				template<typename tuple_t>
				static bool bind(sqlite3_stmt* stmt, const tuple_t& tuple) {
					const auto& val = std::get<0>(tuple);
					return  detail::do_bind(stmt, val, 1);
				}

				template<typename tuple_t, typename array_t>
				static bool bind_para(sqlite3_stmt* stmt, const tuple_t& tuple, array_t&& arr)
				{
					const auto& val = std::get<0>(tuple);
					return detail::do_bind_para(stmt, val, arr[0]);
				}

				template<typename tuple_t>
				static auto retrive(sqlite3_stmt* statement) {
					constexpr size_t col = (std::tuple_size_v<tuple_t> -1);
					using arg_type = std::tuple_element_t<col, tuple_t>;

					return detail::do_retrive<arg_type, 0>(statement);
				}

				template<typename tuple_t>
				static bool build(tuple_t& tup, const pof::base::data::row_t& row)
				{
					auto& v = row.first;
					if (v.empty()) return false;
					using arg_type = std::tuple_element_t<0, tuple_t>;

					if (boost::variant2::holds_alternative<arg_type>(v[0])) {
						std::get<0>(tup) = boost::variant2::get<arg_type>(v[0]);
					}
					else {
						if constexpr (std::is_integral_v<arg_type>) {
							std::get<0>(tup) = static_cast<arg_type>(0);
						}
						else if constexpr (std::is_floating_point_v<arg_type>) {
							std::get<0>(tup) = static_cast<arg_type>(0.0f);
						}
						else {
							std::get<0>(tup) = arg_type{};
						}
					}
					return true;
				}
			};
		}
		


		template<typename... Args, size_t... I>
		pof::base::data::row_t::first_type make_row_from_tuple_impl(const std::tuple<Args...>& tup,
			std::index_sequence<I...>)
		{
			return { (std::move(std::get<I>(tup)))...};
		}

		template<typename tuple, typename idx = std::make_index_sequence<std::tuple_size_v<tuple>>>
		pof::base::data::row_t::first_type make_row_from_tuple(const tuple& tup)
		{
			return make_row_from_tuple_impl(tup, idx{});
		}

		template<typename... Args, size_t... I>
		auto make_tuple_from_row_impl(const pof::base::data::row_t::first_type& row,
				const std::tuple<Args...>& tup, std::index_sequence<I...>) -> std::tuple<Args...>
		{
			return std::make_tuple(boost::variant2::get<Args>(row[I])...);
		}

		template<typename tuple, typename idx = std::make_index_sequence<std::tuple_size_v<tuple>>>
		auto make_tuple_from_row(const pof::base::data::row_t::first_type& row) -> tuple {
			return make_tuple_from_row_impl(row, tuple{}, idx{});
		}

		template<typename... Args>
		bool build(std::tuple<Args...>& tup, const pof::base::data::row_t& row) {
			constexpr size_t count = sizeof...(Args);
			assert(count == row.first.size());

			return detail::loop<count - 1>::build(tup, row);

		}

		struct func_aggregate
		{
			typedef void (*arg_func)(sqlite3_context*, int, sqlite3_value**);
			typedef void(*arg_step)(sqlite3_context*, int, sqlite3_value**);
			typedef void(*arg_final)(sqlite3_context*);
			constexpr const static std::int32_t encoding = SQLITE_UTF8;

			std::string name;
			std::int32_t arg_count = 0;
			void* user_data =  nullptr;
			arg_func func = nullptr;
			arg_step fstep = nullptr;
			arg_final ffinal =  nullptr;

			func_aggregate() = default;
		};


		extern void month_func(sqlite3_context* conn, int arg, sqlite3_value** vals);
		extern void day_func(sqlite3_context* conn, int arg, sqlite3_value** vals);
		//extern void _func(sqlite3_context* conn, int arg, sqlite3_value** vals);
		extern void cost_step_func(sqlite3_context* con, int row, sqlite3_value** vals);
		extern void cost_final_func(sqlite3_context* conn);
		extern void cost_multi_add(sqlite3_context* conn, int arg, sqlite3_value** vals);
		extern void cost_multi(sqlite3_context* conn, int arg, sqlite3_value** vals);


		class database : public boost::noncopyable {
		public:
			using stmt_t = std::add_pointer_t<sqlite3_stmt>;
			using conn_t = std::add_pointer_t<sqlite3_context>;
			using value_arr_t = std::add_pointer_t<sqlite3_value*>;

			using stmt_map = std::unordered_map<std::string, stmt_t>;
			using query_t = std::string;

			typedef int(*exeu_callback)(void* arg, int col, char** rol_val, char** col_names);
			typedef int(*commit_callback)(void* arg);
			typedef void(*rollback_callback)(void* arg);
			typedef void(*update_callback)(void* arg, int evt, char const* database_name, char const* table_name, sqlite_int64 rowid);
			typedef int(*trace_callback)(std::uint32_t traceType, void* UserData, void* statement, void* traceData);
			typedef int(*busy_callback)(void* arg, int i);
			typedef int(*progress_callback)(void* arg);
			typedef int(*auth)(void* arg, int eventCode, const char* evt_1, const char* evt_2, const char* database_name, const char* tig_view_name);


			database(const std::filesystem::path& path);
			~database();
			database(database&& db) noexcept;
			database& operator=(database&& db) noexcept;


			std::optional<stmt_t> prepare(const query_t& query) const;
			std::optional<stmt_t> prepare(std::string_view query) const;
			void reset(stmt_t stmt) const;
			void finalise(stmt_t stmt) const;
			void clear_bindings(stmt_t stmt) const;
			bool add_map(const std::string& name, stmt_t stmt);
			bool remove_map(const std::string& name);
			bool flush_db();
			bool register_func(const func_aggregate& argg);
			std::optional<stmt_t> get_map(const std::string& value);

			void set_commit_handler(commit_callback callback, void* UserData);
			bool set_trace_handler(trace_callback callback, std::uint32_t mask, void* UserData);
			bool set_busy_handler(busy_callback callback, void* UserData);
			void set_rowback_handler(rollback_callback callback, void* UserData);
			void set_update_handler(update_callback callback, void* UserData);
			bool set_auth_handler(auth callback, void* UserData);
			void set_progress_handler(progress_callback callback, void* UserData, int frq);


			bool execute(const query_t& query) const;
			bool execute(stmt_t stmt) const;
			bool roll_back(stmt_t stmt); 

			bool begin_trans() const;
			bool end_trans() const;

			inline std::string_view err_msg() const { return std::string_view(sqlite3_errmsg(m_connection)); }
			inline int err_code() const { return sqlite3_errcode(m_connection); }

			bool backup(const std::filesystem::path& location, const std::function<bool(int)>& progress);
			bool rollback_data(const std::filesystem::path& location, const std::function<bool(int)>& progress);


			template<typename T>
			static void result(conn_t conn, const T& ret)
			{
				static_assert(is_database_type<T>::value, "result type is not a valid database type");
				if constexpr (std::is_integral_v<T>) {
					if constexpr (sizeof(T) == 4) {
						sqlite3_result_int(conn, ret);
					}
					else {
						sqlite3_result_int64(conn, ret);
					}
				}
				else if constexpr (std::is_floating_point_v<T>)
				{
					sqlite3_result_double(conn, ret);
				}
				else if constexpr (std::is_same_v<T, pof::base::data::text_t> || std::is_same_v<T, pof::base::data::blob_t>) {
					if (ret.empty()) {
						sqlite3_result_null(conn);
					}
					else {
						sqlite3_result_text(conn, ret.data(), ret.size(), SQLITE_TRANSIENT);
					}
				}
				else if constexpr (std::is_same_v<T, pof::base::data::datetime_t>)
				{
					pof::base::data::clock_t::rep rep = ret.time_since_epoch().count();
					sqlite3_result_int64(conn, rep);
				}
				else if constexpr (std::is_same_v<T, pof::base::data::duuid_t>) {
					sqlite3_result_blob(conn, reinterpret_cast<void*>(ret.data), ret.size(), SQLITE_TRANSIENT);
				}
				else if constexpr (std::is_same_v<T, pof::base::data::currency_t>) {
					sqlite3_result_blob(conn, ret.data().data(), ret.size(), SQLITE_TRANSIENT);
				
				}
				else if constexpr (std::is_enum_v<T>) {
					sqlite3_result_int(conn, static_cast<int>(ret));
				}
			}

			template<typename T, size_t P = 0>
			static auto arg(conn_t conn, value_arr_t vals) -> T
			{
				static_assert(is_database_type<T>::value, "values type is not a valid database type");
				if constexpr (std::is_integral_v<T>){
					if(sqlite3_value_type(vals[P]) != SQLITE_INTEGER){
						sqlite3_result_error(conn, "INVALID INTEGER TYPE", 500);
					}
					else {
						if constexpr (sizeof(T) == 4) {
							return sqlite3_value_int(vals[P]);
						}
						else {
							return sqlite3_value_int64(vals[P]);
						}
					}
				}
				else if constexpr (std::is_floating_point_v<T>) {
					if (sqlite3_value_type(vals[P]) != SQLITE_FLOAT) {
						sqlite3_result_error(conn, "INVALID FLOATING POINT TYPE", 501);
					}
					else {
						return sqlite3_value_double(vals[P]);
					}
				}
				else if constexpr (std::is_same_v<T, pof::base::data::text_t>) {
					if (sqlite3_value_type(vals[P]) != SQLITE_TEXT) {
						sqlite3_result_error(conn, "INVALID TEXT TYPE", 502);
					}
					else {
						auto ptr = (const char*)sqlite3_value_text(vals[P]);
						if (ptr) {
							 return pof::base::data::text_t(ptr);
						}
						else {
							return pof::base::data::text_t{};
						}
					}
				}
				else if constexpr (std::is_same_v<T, pof::base::data::blob_t>) {
					if (sqlite3_value_type(vals[P]) != SQLITE_BLOB) {
						sqlite3_result_error(conn, "INVALID BLOB TYPE", 503);
					}
					else {
						auto ptr = reinterpret_cast<pof::base::data::blob_t::value_type*>(sqlite3_value_blob(vals[P]));
						if (ptr) {
							const size_t count = sqlite3_value_bytes(vals[P]);
							return pof::base::data::blob_t(ptr, ptr + count);
						}
						else {
							pof::base::data::blob_t{};
						}
					}
				}
				else if constexpr (std::is_same_v<T, pof::base::data::datetime_t>)
				{
					assert(sqlite3_value_type(vals[P]) == SQLITE_INTEGER);

					pof::base::data::datetime_t::duration::rep rep = sqlite3_value_int64(vals[P]);
					return pof::base::data::datetime_t(pof::base::data::clock_t::duration(rep));
				}
				else if constexpr (std::is_same_v<T, pof::base::data::duuid_t>) {
					auto ptr = reinterpret_cast<const pof::base::data::blob_t::value_type*>(sqlite3_value_blob(vals[P]));
					pof::base::data::duuid_t duid = {};
					if (ptr) {
						std::copy(ptr, ptr + duid.size(), duid.begin());
					}
					return duid;
				}
				else if constexpr (std::is_same_v<T, pof::base::data::currency_t>) {
					auto ptr = reinterpret_cast<pof::base::data::blob_t::value_type*>(sqlite3_value_blob(vals[P]));
					pof::base::data::currency_t cur{};
					if (ptr) {
						std::copy(ptr, pof::base::currency::max, cur.data().begin());
					}
					return cur;
				}
				else if constexpr (std::is_enum_v<T>) {
					assert(sqlite3_value_type(vals[P]) == SQLITE_INTEGER);
					return static_cast<T>(sqlite3_value_int(vals[P]));
				}
			}

			template<size_t N, std::enable_if_t<std::cmp_greater(N, 1), int> = 0>
			auto prepare_multiple(std::string_view sql) const -> std::optional<std::array<stmt_t, N>>
			{
				std::array<stmt_t, N> stmts;
				if (sql.empty()) return std::nullopt;
				std::string_view subquery = sql;
				const char* tail = nullptr;
				stmt_t stmt = nullptr;
				size_t count = N;
				while(count--) {
					if (sqlite3_prepare_v2(m_connection, subquery.data(), subquery.size(), &stmt, &tail) != SQLITE_OK) return std::nullopt;
					if (tail == nullptr) break;

					stmts[count] = stmt;
					subquery = std::string_view{ tail, strlen(tail) };
				};
				return stmts;
			}


			template<typename... Args>
			auto retrive(const std::string& name) -> std::optional<pof::base::relation<Args...>>
			{
				using type_list = std::tuple<Args...>;
				auto iter = m_stmap.find(name);
				if (iter == m_stmap.end()) return std::nullopt;
				stmt_t stmt = iter->second;

				return retrive(stmt);
			}

			template<typename... Args>
			auto retrive(stmt_t stmt) -> std::optional<pof::base::relation<Args...>>
			{
				using rel_t = relation<Args...>;
				using type_list = std::tuple<Args...>;
				constexpr const size_t s = sizeof...(Args);

				if (stmt == nullptr) return std::nullopt;
				rel_t optrel;

				if (sqlite3_step(begin) != SQLITE_DONE) {
					//finalise begin?
					sqlite3_step(rollback);
					reset(begin);
					reset(rollback);
					return std::nullopt;
				}
				int ret = 0;
				while((ret = sqlite3_step(stmt)) == SQLITE_ROW)
				{
					auto tup = detail::loop<s - 1>::template retrive<type_list>(stmt);
					optrel.emplace_back(std::move(tup));
				}
				if (ret != SQLITE_DONE) {
					////SOME ERROR OCCURED ? HOW TO HANDLE
					//sqlite3_step(rollback);
					//reset(stmt); //or finalise
					//reset(begin);
					//reset(rollback);
					return std::nullopt;
				}

				if (sqlite3_step(end) != SQLITE_DONE) {
					//finalise end ? 
					sqlite3_step(rollback);
					
					reset(begin);
					reset(end);
					reset(rollback);
					return std::nullopt;
				}

				reset(end);
				reset(stmt);
				reset(begin);
				return optrel;
			}

			template<typename... Args>
			bool bind(stmt_t stmt, const std::tuple<Args...>& args) {
				using tuple_t = std::tuple<Args...>;
				constexpr const size_t s = sizeof...(Args);
				return detail::loop<s - 1>::template bind(stmt, args);
			}

			template<typename T>
			bool bind(stmt_t stmt, const T& value, size_t pos)
			{
				return detail::do_bind(stmt, value, pos);
			}

			template<typename... Args>
			bool bind_para(stmt_t stmt, const std::tuple<Args...>& args, std::array<std::string_view, sizeof...(Args)>&& para)
			{
				using tuple_t = std::tuple<Args...>;
				using array_t = std::array<std::string_view, sizeof...(Args)>;
				constexpr const size_t s = sizeof...(Args);
				return detail::loop<s - 1>::template bind_para(stmt, args, std::forward<array_t>(para));
			}

			//for insert statments that inserts an entire relation
			template<typename... Args>
			bool store(stmt_t stmt, pof::base::relation<Args...>&& rel)
			{
				using type_list = std::tuple<Args...>;
				if (sqlite3_step(begin_immidiate) != SQLITE_DONE) {
					sqlite3_step(rollback);
					reset(begin_immidiate);
					reset(rollback);
					return false;
				}
				for (auto& row : rel) {
					if (!bind(stmt, std::forward<typename pof::base::relation<Args...>::tuple_t>(row))) {
						sqlite3_step(rollback);
						reset(stmt);
						reset(begin_immidiate);
						reset(rollback);
						return false;
					}

					if (sqlite3_step(stmt) != SQLITE_DONE) {
						sqlite3_step(rollback);
						reset(stmt);
						reset(begin_immidiate);
						reset(rollback);
						return false;
					}
					reset(stmt);
					sqlite3_clear_bindings(stmt);
				}

				if (sqlite3_step(end) != SQLITE_DONE) {
					sqlite3_step(rollback);
					reset(stmt);
					reset(begin_immidiate);
					reset(end);
					reset(rollback);
					return false;
				}
				reset(end);
				reset(stmt);
				reset(begin_immidiate);
				return true;
			}

			//for update statements 
			template<typename... Args>
			bool update(stmt_t stmt,
				std::array<std::string_view, sizeof...(Args)>&& paras, pof::base::relation<Args...>&& rel)
			{
				if (sqlite3_step(begin_immidiate) != SQLITE_DONE) {
					sqlite3_step(rollback);
					reset(begin_immidiate);
					reset(rollback);
					return false;
				}
				for (auto& row : rel) {
					if (!bind(stmt, std::forward<pof::base::relation<Args...>>(row), 
							std::forward<std::array<std::string_view, sizeof...(Args)>>(paras))) {
						sqlite3_step(rollback);
						reset(stmt);
						reset(begin_immidiate);
						reset(rollback);
						return false;
					}

					if (sqlite3_step(stmt) != SQLITE_DONE) {
						sqlite3_step(rollback);
						reset(stmt);
						reset(begin_immidiate);
						reset(rollback);
						return false;
					}
				}

				if (sqlite3_step(end) != SQLITE_DONE) {
					sqlite3_step(rollback);
					reset(stmt);
					reset(begin_immidiate);
					reset(end);
					reset(rollback);
					return false;
				}
				reset(end);
				reset(stmt);
				reset(begin_immidiate);
				return true;
			}
		private:
			sqlite3* m_connection;
			stmt_map m_stmap;

			stmt_t begin = nullptr;
			stmt_t begin_immidiate = nullptr;
			stmt_t end = nullptr;
			stmt_t rollback = nullptr;

		};
	
	};
};