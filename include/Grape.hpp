#pragma once
#include "serialiser.h"
#include <boost/fusion/include/define_struct.hpp>
#include "net.h"

template<size_t N>
using opt_field_string = grape::optional_field<std::string, N>;

template<size_t N>
using opt_field_uint64_t = grape::optional_field<std::uint64_t, N>;

template<size_t N>
using opt_field_time_point = grape::optional_field<std::chrono::system_clock::time_point, N>;

template<size_t N>
using opt_field_currency = grape::optional_field<pof::base::currency, N>;

template<size_t N>
using opt_field_uuid = grape::optional_field<boost::uuids::uuid, N>;

//tree drag and drop data
BOOST_FUSION_DEFINE_STRUCT(
	(ab), treeDnd,
	(size_t, id)
	(void*, win)
	(std::string, name)
	(int, img)
)

//institution
BOOST_FUSION_DEFINE_STRUCT(
	(grape), institution,
	(boost::uuids::uuid, id)
	(std::string, name)
	(std::uint8_t, type)
	(boost::uuids::uuid, address_id)
	(std::string, info)
)

//pharmacy
BOOST_FUSION_DEFINE_STRUCT(
	(grape), pharmacy,
	(boost::uuids::uuid, id)
	(std::string, name)
	(boost::uuids::uuid, address_id)
	(std::string, info)
)

namespace grape {
	enum class branch_type : std::uint32_t{
		community,
		hospital,
		industry,
		drf,
		educational
	};

	enum class branch_state : std::uint32_t {
		open,
		closed,
		shutdown,
	};
};

//branches
BOOST_FUSION_DEFINE_STRUCT(
	(grape), branch,
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, address_id)
	(std::string, name)
	(grape::branch_type, type)
	(grape::branch_state, state)
	(std::string, info)
)

//branch collection
BOOST_FUSION_DEFINE_STRUCT(
	(grape)(collection),
	branches,
	(std::vector<grape::branch>, group)
)

//operation result message
BOOST_FUSION_DEFINE_STRUCT(
	(grape), result,
	(std::string, status)
	(std::string, message)
)

//pharmacy credentials
BOOST_FUSION_DEFINE_STRUCT(
	(grape), credentials,
	(boost::uuids::uuid, account_id)
	(boost::uuids::uuid, session_id)
	(boost::uuids::uuid, pharm_id)
	(boost::uuids::uuid, branch_id)
)

//app details
BOOST_FUSION_DEFINE_STRUCT(
	(grape), app_details,
	(boost::uuids::uuid, app_id)
	(boost::uuids::uuid, app_install_location_id)
	(std::string, app_name)
	(std::string, app_version)
	(std::string, os)
	(std::string, locale)
	(std::chrono::system_clock::time_point, app_installed_date)
	(std::chrono::system_clock::time_point, app_last_update)
	(std::chrono::system_clock::time_point, app_last_ping)
)

//file buffer
BOOST_FUSION_DEFINE_STRUCT(
	(grape), file,
	(std::string, name)
	(std::vector<std::uint8_t>, content)
)

//address
BOOST_FUSION_DEFINE_STRUCT(
	(grape), address,
	(boost::uuids::uuid, id)
	(std::string, country)
	(std::string, state)
	(std::string, lga)
	(std::string, street)
	(std::string, num)
	(std::string, add_info)
)


BOOST_FUSION_DEFINE_STRUCT(
	(grape), page,
	(std::uint32_t, begin)
	(std::uint32_t, limit)
)

namespace grape {
	//formulary access level
	enum class formulary_access_level : std::uint32_t {
		ACCESS_PRIVATE = 0x01,
		ACCESS_PUBLIC = 0x02,
	};

	//order state
	enum class order_state : std::uint32_t {
		PENDING,
		ORDERED,
		COMPLETED,
		DELIVERED,
	};

	//warning level
	enum class warning_level : std::uint32_t {
		SIMPLE,
		CRITICAL,
	};

	//actions
	enum class action :std::uint32_t
	{
		STOCK_CHECKED,
		BROUGHT_FORWARD,
		CHECK_TIME,
		DATA_BACKUP,
	};
};


BOOST_FUSION_DEFINE_STRUCT(
	(grape), product,
	(boost::uuids::uuid, id)
	(std::uint64_t, serial_num)
	(std::string, name)
	(std::string, generic_name)
	(std::string, class_)
	(std::string, formulation)
	(std::string, strength)
	(std::string, strength_type)
	(std::string, usage_info)
	(std::string, description)
	(std::string, indications)
	(std::uint64_t, package_size)
	(std::string, sideeffects)
	(std::string, barcode)
	(std::string, manufactures_name)
)


BOOST_FUSION_DEFINE_STRUCT(
	(grape), product_opt,
	(boost::uuids::uuid, id)
	(std::uint64_t, serial_num)
	(grape::opt_fields, fields)
	(opt_field_string<0>, name)
	(opt_field_string<1>, generic_name)
	(opt_field_string<2>, class_)
	(opt_field_string<3>, formulation)
	(opt_field_string<4>, strength)
	(opt_field_string<5>, strength_type)
	(opt_field_string<6>, usage_info)
	(opt_field_string<7>, description)
	(opt_field_string<8>, indications)
	(opt_field_uint64_t<9>, package_size)
	(opt_field_string<10>, sideeffects)
	(opt_field_string<11>, barcode)
	(opt_field_string<12>, manufactures_name)
)

//inventory
BOOST_FUSION_DEFINE_STRUCT(
	(grape), inventory,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, product_id)
	(std::chrono::system_clock::time_point, expire_date)
	(std::chrono::system_clock::time_point, input_date)
	(std::uint64_t, stock_count)
	(pof::base::currency, cost)
	(boost::uuids::uuid, supplier_id)
	(std::string, lot_number)
)

//packs
BOOST_FUSION_DEFINE_STRUCT(
	(grape), pack,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, product_id)

)

//pharma products
BOOST_FUSION_DEFINE_STRUCT(
	(grape), pharma_product,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, product_id)
	(pof::base::currency, unitprice)
	(pof::base::currency, costprice)
	(std::uint64_t, stock_count)
	(std::uint64_t, min_stock_count)
	(std::chrono::system_clock::time_point, date_added)
	(std::chrono::system_clock::time_point, date_expired)
	(std::uint64_t, category_id)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape), pharma_product_opt,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, product_id)
	(grape::opt_fields, fields)
	(opt_field_currency<0>, unitprice)
	(opt_field_currency<1>, costprice)
	(opt_field_uint64_t<2>, stock_count)
	(opt_field_uint64_t<3>, min_stock_count)
	(opt_field_time_point<4>, date_added)
	(opt_field_time_point<5>, date_expired)
	(opt_field_uint64_t<6>, category_id)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape), product_identifier,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, product_id)
)

//collections
BOOST_FUSION_DEFINE_STRUCT(
	(grape)(collection), products,
	(std::vector<grape::product>, group)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape)(collection), pharma_products,
	(std::vector<grape::pharma_product>, group)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape), formulary,
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, creator_id)
	(std::string, name)
	(std::string, created_by)
	(std::chrono::system_clock::time_point, created_date)
	(std::string, version)
	(grape::formulary_access_level, access_level)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape), order,
	(grape::order_state, state)
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, product_id)
	(pof::base::currency, total_cost)
	(std::uint64_t, quantity)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape), warnings,
	(grape::warning_level, state)
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, product_id)
	(std::string, warning_text)
)


BOOST_FUSION_DEFINE_STRUCT(
	(grape), category,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(std::uint64_t, category_id)
	(std::string, name)
)

BOOST_FUSION_DEFINE_STRUCT(
	(grape), expired,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, branch_id)
	(boost::uuids::uuid, product_id)
	(std::chrono::system_clock::time_point, expired_date)
	(std::uint64_t, stock_count)
)

namespace grape {
	using opt_hash = grape::optional_field<std::string, 0>;
	using opt_secque = grape::optional_field<std::string, 1>;
	using opt_secans = grape::optional_field<std::string, 2>;
	using opt_sessionid = grape::optional_field<boost::uuids::uuid, 3>;
	using opt_session_start_time = grape::optional_field<std::chrono::system_clock::time_point, 4>;

	enum class account_type : std::uint32_t {
		pharmacist,
		loccum_pharmacist,
		intern_pharmacist,
		pharmacy_tech,
		student_pharmacist,
		dispenser,
		sale_assistant
	};
};

//accounts
BOOST_FUSION_DEFINE_STRUCT(
	(grape), account,
	(boost::uuids::uuid, pharmacy_id)
	(boost::uuids::uuid, account_id)
	(grape::account_type, type)
	(std::bitset<5>, privilage)
	(std::string, first_name)
	(std::string, last_name)
	(std::chrono::year_month_day, dob)
	(std::string, phonenumber)
	(std::string, email)
	(std::string, username)
	(grape::opt_fields, fields)
	(grape::opt_hash, passhash)
	(grape::opt_secque, sec_que)
	(grape::opt_secans, sec_ans)
	(grape::opt_sessionid, session_id)
	(grape::opt_session_start_time, session_start_time)
)

//accounts collection
BOOST_FUSION_DEFINE_STRUCT(
	(grape)(collection), accounts,
	(std::vector<grape::account>, group)
)

//account cred
BOOST_FUSION_DEFINE_STRUCT(
	(grape), account_cred,
	(boost::uuids::uuid, pharmacy_id)
	(std::string, username)
	(std::string, password)
	(std::chrono::system_clock::time_point, last_session_time)
)

//account sign in reponse, a session cred
BOOST_FUSION_DEFINE_STRUCT(
	(grape), session_cred,
	(std::chrono::system_clock::time_point, session_start_time)
	(boost::uuids::uuid, session_id)
)


namespace grape {
	template<typename T>
		requires grape::FusionStruct<T>
	using collection_type = boost::fusion::vector<std::vector<T>>;
	using date_query_t = boost::fusion::vector<opt_fields, optional_field<std::chrono::year_month_day, 0>>;
	using pid = grape::collection_type<boost::fusion::vector<boost::uuids::uuid>>;
	using optional_list_t = boost::fusion::vector<opt_fields, optional_field<std::vector<boost::uuids::uuid>, 0>>;
	using string_t = boost::fusion::vector<std::string>;
	using uid_t = boost::fusion::vector<boost::uuids::uuid>;

	using session = pof::base::ssl::session<
		http::vector_body<std::uint8_t>,
		http::vector_body<std::uint8_t>
	>;
};
