#include "errc.h"

static std::unique_ptr<pof::base::error_category> g_error_category;
static std::once_flag g_flag;

pof::base::error_category::error_category() {
	//PREPARE THE ERROR CODES MESSAGES
	m_messages[to_underlying(errc::no_data)] = "NO DATA IN BUFFER";
	m_messages[to_underlying(errc::metadata_mismatch)] = "DATA IN ROW DOES NOT MATCH THE METADATA FOR DATA OBJECT";
	m_messages[to_underlying(errc::no_database_hostname)] = "NO DATABASE HOSTNAME";
	m_messages[to_underlying(errc::no_arguments)] = "NO ARGUMENTS IN STMT";
	m_messages[to_underlying(errc::no_connection_avaliable)] = "NO CONNECTION AVAILABLE";

}

std::string pof::base::error_category::message(int condition) const
{
	return m_messages[condition];
}

const char* pof::base::error_category::name() const noexcept
{
	return "PHARMAOFFICE_ERROR_CATEGORY";
}

std::error_condition pof::base::error_category::default_error_condition(int code) const noexcept
{
	return std::error_condition(code, get_err_category());
}


const pof::base::error_category& pof::base::get_err_category() noexcept
{
	std::call_once(g_flag, []() {
		g_error_category = std::make_unique<pof::base::error_category>();
	});
	return *g_error_category;
}

std::error_code std::make_error_code(pof::base::errc e) noexcept
{
	return std::error_code(pof::base::to_underlying(e), pof::base::get_err_category());
}