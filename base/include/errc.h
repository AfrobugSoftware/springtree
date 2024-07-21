#pragma once
#include <system_error>
#include <exception>
#include <array>
#include <type_traits>
#include <string>

#include <memory>
#include <mutex>

#include <fmt/format.h>

template<>
class fmt::formatter<std::error_code>
{
public:
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		auto it = ctx.begin(), end = ctx.end();
		if (it != end &&
			*it == 'e' && *(++it) == 'c') {
			it++; //should point to }
		}
		if (it == end && *it != '}') {
			throw fmt::format_error("error code format specifyer invalid");
		}
		return it;
	}

	template <typename FormatContext>
	auto format(const std::error_code& p, FormatContext& ctx) const -> decltype(ctx.out()) {
		return fmt::format_to(ctx.out(), "{}: {:d}", p.message(), p.value());
	}
};

namespace pof {
	namespace base {
		enum class errc {
			no_data,
			metadata_mismatch,
			no_database_hostname,
			no_arguments,
			no_connection_avaliable,
			//DO NOT INSERT AFTER THIS, MAX ERROR COUNT
			max_size
		};

		constexpr std::underlying_type_t<errc> to_underlying(errc ec) noexcept {
			return static_cast<std::underlying_type_t<errc>>(ec);
		}

		class error_category : public std::error_category {
		public:
			error_category();
			virtual ~error_category() {}
			virtual std::string message(int condition) const override;
			virtual const char* name() const noexcept override;
			virtual std::error_condition default_error_condition(int code) const noexcept;
		private:
			std::array<std::string, pof::base::to_underlying(errc::max_size)> m_messages;
		};
		extern const error_category& get_err_category() noexcept;

	};
};

namespace std {
	template<>
	struct is_error_code_enum<pof::base::errc> : std::true_type {};
	std::error_code make_error_code(pof::base::errc code) noexcept;
};