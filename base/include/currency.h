#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>
#include <locale>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>

using namespace std::literals::string_literals;
namespace pof {
	namespace base {

		class currency_exception : public std::exception
		{
		public:
			currency_exception(const std::string& re): reason(re) {}
			virtual const char* what() const override {
				return reason.c_str();
			}
		private:
			std::string reason;
		};

		class currency {
		public:
			//14.2 14 char for the higher domination and 2 for the lower
			//99,999,999,999,999.99 the highest amount a currency can hold
			constexpr static size_t max = 17;
			constexpr static const std::string_view cur_type = "N";
			using cur_char_t = char;
			using cur_t = std::array <cur_char_t, max>;


			currency();
			
			explicit currency(const std::string& cur);
			explicit currency(double cur);
			explicit currency(const cur_t& cur_data);
			
			currency(const currency& rhs);
			currency(currency&& rhs) noexcept;

			currency& operator=(const currency& rhs);
			currency& operator=(currency&& rhs) noexcept;
			currency& operator=(double rhs);

			currency operator/(double denomiator) const;
			currency operator+(const currency& rhs) const;
			currency operator+(double rhs) const;
			currency operator-(const currency & rhs) const;
			currency operator-(double rhs) const;
			currency operator*(double rhs) const;
			
			//update 
			currency& operator+=(const currency& rhs);
			currency& operator-=(const currency& rhs);
			currency& operator/=(double rhs);
			currency& operator*=(double rhs);

			//relation operation
			bool operator==(const currency& rhs) const;
			bool operator<(const currency& rhs) const;
		

			//type conversion
			operator std::string() const;
			operator double() const;

			//literal

			inline constexpr const cur_t& data() const { return m_data; }
			inline constexpr cur_t& data() { return m_data; }
		private:
			cur_t m_data = {0};

		};	
		currency operator""_cu(long double fig);
		class  curlocale : public std::numpunct<char>
		{
		protected:
			virtual char do_thousands_sep()   const override { return ','; }  // separate with comma
			virtual std::string do_grouping() const override { return "\03"s; } // groups of 3 digits
		};
	};

};

#include <fmt/format.h>
template<>
class fmt::formatter<pof::base::currency>
{
public:
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		auto it = ctx.begin(), end = ctx.end();
		if (it != end &&
			*it == 'c' && *(++it) == 'u') {
			it++; //should point to }
		}
		if (it == end && *it != '}') {
			throw fmt::format_error("error code format specifyer invalid");
		}
		return it;
	}

	template <typename FormatContext>
	auto format(const pof::base::currency& p, FormatContext& ctx) const -> decltype(ctx.out()) {
		static std::stringstream os;
		static std::locale local(std::locale(), new pof::base::curlocale);
		static bool bue = false;
		if (!bue) { os.imbue(local); bue = true; }

		os << std::setprecision(2) << std::fixed << static_cast<double>(p);
		auto out = fmt::format_to(ctx.out(), "{} {}", pof::base::currency::cur_type, os.str());
		os.str(std::string());
		return out;
	}
};