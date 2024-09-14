#pragma once
#include <boost/asio/buffer.hpp>
#include <boost/fusion/include/adapter.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/algorithm/transformation.hpp>
#include <boost/fusion/include/transformation.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/mysql/field.hpp>

#include "Data.h"
#include "currency.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <bit>
#include <bitset>

namespace boost {
	namespace fusion {
		namespace traits {
			template <>
			struct is_view<std::chrono::system_clock::time_point> : mpl::false_ {};
		}
	}
}

namespace grape
{


	template<typename T>
	concept Integers = std::is_integral_v<T>  || std::is_floating_point_v<T>;

	template<typename T>
	concept Pods = std::is_pod_v<T> && !Integers<T> && !std::is_enum_v<T>;

	template<typename T>
	concept Enums = std::is_enum_v<T>;

	template<typename T>
	concept FusionStruct = boost::mpl::is_sequence<T>::value;

	template<typename T>
	concept Pointer = std::is_pointer_v<T>;

	template<Integers T>
	constexpr auto bswap(const T& value) -> T {
		auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
		std::ranges::reverse(value_representation);
		return std::bit_cast<T>(value_representation);
	}

	template<typename T>
	concept Hashable = requires(T a)
	{
		{ boost::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
	};

	template<typename T, size_t N = CHAR_BIT * sizeof(T)>
	struct optional_field_set {
		using value_type = T;
		using bits_type = std::bitset<N>;
	};

	template<typename T, size_t N>
	struct optional_field : std::optional<T> {
		constexpr static const size_t bit = N;
		using value_type = T;
		using std::optional<T>::operator=;
	};

	using opt_fields = optional_field_set<std::uint16_t>;

	template<typename T>
	class is_optional_field : public std::false_type {};

	template<typename T, size_t N>
	class is_optional_field<optional_field<T, N>> : public std::true_type {};

	template<typename T>
	class is_optional_field_set : public std::false_type {};

	template<>
	class is_optional_field_set<opt_fields> : public std::true_type {};


	namespace serial {
		class reader {
		public:
			mutable boost::asio::const_buffer buf_;
			mutable std::optional<opt_fields::bits_type> opt_;

			reader(boost::asio::const_buffer buf) : buf_{ std::move(buf) }, opt_{std::nullopt} {}
			
			template<Integers T>
			void operator()(T& i) const {
				i = bswap(*boost::asio::buffer_cast<const T*>(buf_));
				buf_ += sizeof(T);
			}

			template<Enums E>
			void operator()(E& e) const {
				typename std::underlying_type<E>::type v;
				(*this)(v);
				e = static_cast<E>(v);
			}

			template<size_t N>
				requires (std::integral_constant<int, N>::value <= 32)
			void operator()(std::bitset<N>& bs) const {
				std::uint32_t val = 0;
				(*this)(val);
				bs = std::bitset<N>(val);
			}

			/*template<Pointer T>
			void operator()(T& p) const {
				p = bswap(*boost::asio::buffer_cast<const T*>(buf_));
				buf_ += sizeof(T);
			}*/

			template<Hashable K, typename T>
			void operator()(boost::unordered_flat_map<K, T>& map) const {
				std::uint32_t len = 0;
				(*this)(len);
				for (; len; len--){
					K key{};
					(*this)(key);
					T val{};
					(*this)(val);
					map.emplace(key, val);
				}
			}

			void operator()(std::chrono::year_month_day& ymd) const {
				const std::uint32_t i = bswap(*boost::asio::buffer_cast<const std::uint32_t*>(buf_));
				const short y = (i & 0xFFFF0000) >> 16;
				const char m =  (i & 0x0000FF00) >> 8;
				const char d =  i & 0x000000FF;
				ymd = std::chrono::year_month_day(std::chrono::year(y), std::chrono::month(m), std::chrono::day(d));
				
				buf_ += sizeof(std::uint32_t);
			}

			void operator()(std::chrono::system_clock::time_point& tp) const {
				tp = std::chrono::system_clock::time_point{
					std::chrono::system_clock::duration{
					bswap(*boost::asio::buffer_cast<const std::chrono::system_clock::rep*>(buf_))}};
				buf_ += sizeof(std::chrono::system_clock::rep);
			}

			void operator()(boost::uuids::uuid& uuid) const {
				memcpy(uuid.data, buf_.data(), uuid.static_size());
				buf_ += uuid.static_size();
			}

			void operator()(std::string& str) const {
				std::uint32_t len = 0;
				(*this)(len);
				str = std::string(reinterpret_cast<const char*>(buf_.data()), len);
				buf_ += len;
			}	

			void operator()(pof::base::currency& cur) const {
				memcpy(cur.data().data(), buf_.data(), cur.data().size());
				buf_ += cur.data().size();
			}
			template<Pods P>
			void operator()(P& p) const {
				memcpy(&p, buf_.data(), sizeof(P));
				buf_ += sizeof(P);
			}

			template<size_t N>
			void operator()(std::array<char, N>& fixed) const {
				memcpy(fixed.data(), buf_.data(), N);
				buf_ += N;
			}

			template<FusionStruct T>
			void operator()(T& val) const {
				boost::fusion::for_each(val, [&](auto& i) {
					(*this)(i);
				});
			}
			template<typename T, size_t N>
			void operator()(optional_field_set<T,N>& f) const {
				opt_fields::value_type val;
				(*this)(val);
				opt_ = opt_fields::bits_type(val);
			}

			template<class T, size_t N>
			void operator()(optional_field<T, N>& val) const {
				if (!opt_.has_value()) throw std::logic_error("optional field comes before optional set");;
				if ((*opt_)[N]) {
					T v{};
					(*this)(v);
					val = v;
				}
			}

			template<typename T> 
				requires Integers<T> || FusionStruct<T> || Enums<T> || Pods<T>
			void operator()(std::vector<T>& vec) const
			{
				std::uint32_t len = 0;
				(*this)(len);
				vec.reserve(len);
				if constexpr (std::is_integral_v<T>) {
					std::copy(reinterpret_cast<T*>(buf_.data()),
						reinterpret_cast<T*>(buf_.data()) + len, vec.begin());
				}
				else {
					for (; len; --len) {
						T v{};
						(*this)(v);
						vec.emplace_back(std::move(v));
					}
				}
			}
		};

		class writer {
		public:
			mutable boost::asio::mutable_buffer buf_;
			mutable opt_fields::bits_type opt_;
			mutable opt_fields::value_type* optv_;

			writer(boost::asio::mutable_buffer buf) : buf_{ std::move(buf) }, optv_{ nullptr} {}
			
			template<Integers T>
			void operator()(const T& i) const {
				*boost::asio::buffer_cast<T*>(buf_) = bswap(i);
				buf_ += sizeof(T);
			}

			template<size_t N>
				requires (std::integral_constant<int, N>::value <= 32)
			void operator()(const std::bitset<N>& bs) const {
				std::uint32_t val = bs.to_ulong();
				(*this)(val);
			}

			/*template<Pointer T>
			void operator()(const T& p) const {
				*boost::asio::buffer_cast<T*>(buf_) = bswap(p);
				buf_ += sizeof(T);
			}*/

			template<Enums E>
			void operator()(E& e) const {
				auto i = static_cast<std::underlying_type_t<E>>(e);
				(*this)(i);
			}

			void operator()(const std::chrono::year_month_day& ymd) const {
				const std::uint16_t y = static_cast<int>(ymd.year());
				const std::uint16_t dm = (static_cast<std::uint16_t>((static_cast<unsigned>(ymd.month()))) << 8) | 
					 static_cast<std::uint16_t>(static_cast<unsigned>(ymd.day()));
				const std::uint32_t tt = static_cast<std::uint32_t>(y)  << 16 | static_cast<std::uint32_t>(dm);
				*boost::asio::buffer_cast<std::uint32_t*>(buf_) = bswap(tt);
				buf_ += sizeof(std::uint32_t);
			}

			void operator()(const std::chrono::system_clock::time_point& tp) const {
				*boost::asio::buffer_cast<std::chrono::system_clock::rep*>(buf_)
					= bswap(tp.time_since_epoch().count());
				buf_ += sizeof(std::chrono::system_clock::rep);
			}

			void operator()(const boost::uuids::uuid& uuid)  const {
				memcpy(buf_.data(), uuid.data, uuid.static_size());
				buf_ += uuid.static_size();
			}

			void operator()(const std::string& str) const {
				(*this)(static_cast<std::uint32_t>(str.size()));
				memcpy(buf_.data(), str.data(), str.size());
				buf_ += str.size();
			}

			void operator()(const pof::base::currency& cur) const {
				memcpy(buf_.data(), cur.data().data(), cur.data().size());
				buf_ += cur.data().size();
			}
			template<Pods P>
			void operator()(const P& p) const {
				memcpy(buf_.data(), &p, sizeof(P));
				buf_ += sizeof(P);
			}

			template<size_t N>
			void operator()(const std::array<char, N>& fixed) const {
				memcpy(buf_.data(), fixed.data(), fixed.size());
				buf_ += N;
			}

			template<FusionStruct T>
			void operator()(const T& val) const {
				boost::fusion::for_each(val, [&](const auto& i) {
					(*this)(i);
				});
			}

			template<typename T, size_t N>
			void operator()(const optional_field_set<T, N>& t) const {
				opt_.reset();
				optv_ = boost::asio::buffer_cast<opt_fields::value_type*>(buf_);
				buf_ += sizeof(opt_fields::value_type);
			}
			template<typename T, size_t N>
			void operator()(const optional_field<T, N>& field) const {
				if (optv_ == nullptr) throw std::logic_error("optional field comes before optional set");
				if (field.has_value()) {
					opt_.set(N);
					*optv_ = bswap(static_cast<opt_fields::value_type>(opt_.to_ulong()));
					(*this)(*field);
				}
			}
			template<typename T>
				requires Integers<T> || FusionStruct<T> || Enums<T> || Pods<T>
			|| std::is_same_v<T, std::string> || std::is_array_v<T> || std::is_same_v<T, std::chrono::system_clock::time_point>
			void operator()(const std::vector<T>&vec) const
			{
				if constexpr (std::is_integral_v<T>) {
					if (vec.size() > buf_.size()) throw std::logic_error("Cannot fit content in buffer");
					(*this)(static_cast<std::uint32_t>(vec.size()));

					std::copy(vec.begin(), vec.end(), reinterpret_cast<T*>(buf_.data()));
					buf_ += (vec.size() * sizeof(T));
				}
				else {
					(*this)(static_cast<std::uint32_t>(vec.size()));
					for (const auto& v : vec) {
						(*this)(v);
					}
				}
			}

			template<Hashable H, typename T>
			void operator()(const boost::unordered_flat_map<H, T>& map) const {
				(*this)(static_cast<std::uint32_t>(map.size()));
				for (auto& i : map) {
					(*this)(i.first);
					(*this)(i.second);
				}
			}
		
		};

		class sizer {
		public:
			mutable size_t size = 0;
			constexpr sizer() {}

			template<Integers T>
			constexpr void operator()(const T& i) const {
				size += sizeof(T);
			}

			template<size_t N>
				requires (std::integral_constant<int, N>::value <= 32)
			constexpr void operator()(const std::bitset<N>& bs) const {
				size += sizeof(std::uint32_t);
			}

			/*template<Pointer T>
			constexpr void operator()(const T& i) const {
				size += sizeof(T);
			}*/

			template<Enums T>
			constexpr void operator()(const T& i) const {
				using htype = std::underlying_type_t<std::decay_t<T>>;
				size += sizeof(htype);
			}

			constexpr void operator()(const std::chrono::year_month_day& ymd) const {
				size += sizeof(std::uint32_t);
			}

			constexpr void operator()(const std::chrono::system_clock::time_point& tp) const {
				size += sizeof(std::chrono::system_clock::rep);
			}

			constexpr void operator()(const boost::uuids::uuid& uuid)  const {
				size += uuid.static_size();
			}

			constexpr void operator()(const std::string& str) const {
				size += sizeof(std::uint32_t);
				size += str.size();
			}

			constexpr void operator()(const pof::base::currency& cur) const {
				size += cur.data().size();
			}
			template<Pods P>
			constexpr void operator()(const P& p) const {
				size += sizeof(P);
			}

			template<size_t N>
			constexpr void operator()(const std::array<char, N>& fixed) const {
				size += N;
			}

			template<FusionStruct T>
			constexpr void operator()(const T& val) const {
				boost::fusion::for_each(val, [&](const auto& i) {
					(*this)(i);
				});
			}

			template<typename T, size_t N>
			constexpr void operator()(const optional_field_set<T,N>& t) const {
				size += sizeof(opt_fields::value_type);
			}

			template<typename T, size_t N>
			constexpr void operator()(const optional_field<T, N>&field) const {
				if (field.has_value()) {
					(*this)(field.value());
				}
			}

			template<typename T>
				requires Integers<T> || FusionStruct<T> || Enums<T> || Pods<T>
			|| std::is_same_v<T, std::string> || std::is_array_v<T> || std::is_same_v<T, std::chrono::system_clock::time_point>
				constexpr void operator()(const std::vector<T>&vec) const
			{
				using type = std::decay_t<T>;
				//the space for the size of the vector
				size += sizeof(std::uint32_t);
				if constexpr (std::disjunction_v<std::is_same<type, std::string>,
				  boost::mpl::is_sequence<T>>) {
					for (auto& v : vec) {
						(*this)(v);
					}
				}
				else if constexpr (std::disjunction_v<std::is_integral<T>, std::is_floating_point<T>,
				 std::is_enum<T>, std::is_pod<T>, std::is_array<T>>) {
					size += vec.size() * sizeof(T);
				}
			}

			template<Hashable H, typename T>
			constexpr void operator()(const boost::unordered_flat_map<H, T>& map) const {
				size += sizeof(std::uint32_t);
				for (auto& i : map) {
					(*this)(i.first);
					(*this)(i.second);
				}
			}
		};

	
		template<typename T>
			requires FusionStruct<T>
		auto build(const pof::base::data::row_t::first_type& row) -> T {
			typedef boost::mpl::range_c<unsigned, 0, boost::mpl::size<T>::value> range;
			T ret{};
			boost::fusion::for_each(range(), [&](auto i) {
				using constant = std::decay_t<decltype(i)>;
				using arg_type = std::decay_t<decltype(boost::fusion::at<constant>(ret))>;
				if constexpr (grape::is_optional_field_set<arg_type>::value) {
					//skip types with feild_set
				}
				else if constexpr (grape::is_optional_field<arg_type>::value) {
					using get_type = typename arg_type::value_type;
					if (boost::variant2::holds_alternative<get_type>(row[constant::value])) {
						boost::fusion::at<constant>(ret).value() =
							std::move(boost::variant2::get<get_type>(row[constant::value]));
					}
				}
				else if constexpr (std::is_enum_v<arg_type>) {
					using htype = std::underlying_type_t<arg_type>;
					if (boost::variant2::holds_alternative<htype>(row[constant::value])) {
						boost::fusion::at<constant>(ret) =
							static_cast<arg_type>(std::move(boost::variant2::get<htype>(row[constant::value])));
					}
				}
				else{
					if (boost::variant2::holds_alternative<arg_type>(row[constant::value])) {
						boost::fusion::at<constant>(ret) =
							std::move(boost::variant2::get<arg_type>(row[constant::value]));
					}
				}
			});
			return ret;
		}
			
		template<typename... Args>
		void compose(boost::fusion::vector<Args...>& vec, const pof::base::data::row_t::first_type& row)
		{
			using vector_type = boost::fusion::vector<Args...>;
			typedef boost::mpl::range_c<unsigned, 0, boost::mpl::size<vector_type>::value> range;

			boost::fusion::for_each(range(), [&](auto i) {
				using constant = std::decay_t<decltype(i)>;
				using arg_type = std::decay_t<decltype(boost::fusion::at<constant>(vec))>;
				boost::fusion::at<constant>(vec) = 
					std::move(boost::variant2::get<arg_type>(row[constant::value]));
			});
		}

		template<typename T> requires FusionStruct<T>
		auto make_mysql_arg(const T& f) -> std::vector<boost::mysql::field>
		{
			std::vector<boost::mysql::field> ret;
			ret.reserve(boost::mpl::size<T>::value);
			auto func_opt = [&](auto& t) {
				using type = std::decay_t<decltype(t)>;
				if constexpr (std::disjunction_v<std::is_same<type, boost::uuids::uuid>,
					std::is_array<type>>) {
					ret.emplace_back(boost::mysql::field(boost::mysql::blob(t.begin(), t.end())));
				}
				else if constexpr (std::is_same<type, pof::base::currency>::value) {
					ret.emplace_back(boost::mysql::field(boost::mysql::blob(t.data().begin(), t.data().end())));
				}
				else if constexpr (std::is_same_v<type, std::chrono::system_clock::time_point>) {
					ret.emplace_back(boost::mysql::field(boost::mysql::datetime(std::chrono::time_point_cast<
						boost::mysql::datetime::time_point::duration>(t))));
				}
				else if constexpr (std::is_enum_v<type>) {
					ret.emplace_back(boost::mysql::field(static_cast<std::underlying_type_t<type>>(t)));
				}
				else if constexpr (grape::is_optional_field_set<type>::value) {}
				else {
					ret.emplace_back(boost::mysql::field(t));
				}
			};

			auto func = [&](auto& t) {
				using type = std::decay_t<decltype(t)>;
				if constexpr (std::disjunction_v<std::is_same<type, boost::uuids::uuid>,
					std::is_array<type>>) {
					ret.emplace_back(boost::mysql::field(boost::mysql::blob(t.begin(), t.end())));
				}
				else if constexpr (std::is_same<type, pof::base::currency>::value) {
					ret.emplace_back(boost::mysql::field(boost::mysql::blob(t.data().begin(), t.data().end())));
				}
				else if constexpr (std::is_same_v<type, std::chrono::system_clock::time_point>) {
					ret.emplace_back(boost::mysql::field(boost::mysql::datetime(std::chrono::time_point_cast<
						boost::mysql::datetime::time_point::duration>(t))));
				}
				else if constexpr (std::is_enum_v<type>) {
					ret.emplace_back(boost::mysql::field(static_cast<std::underlying_type_t<type>>(t)));
				}
				else if constexpr (grape::is_optional_field_set<type>::value) {}
				else if constexpr (grape::is_optional_field<type>::value) {
					if (t.has_value()) func_opt(t.value());
				}
				else {
					ret.emplace_back(boost::mysql::field(t));
				}
			};

			boost::fusion::for_each(f, func);
			return ret;
		}

		template<FusionStruct T>
		std::pair<T, boost::asio::const_buffer> read(boost::asio::const_buffer b) {
			reader r(std::move(b));
			T res{};
			boost::fusion::for_each(res, [&](auto& i) {
				r(i);
			});
			return std::make_pair(res, r.buf_);
		}

		template<FusionStruct T>
		boost::asio::mutable_buffer write(boost::asio::mutable_buffer buf, const T& val) {
			writer w(std::move(buf));
			boost::fusion::for_each(val, [&](const auto& i) {
				w(i);
			});
			return w.buf_;
		}

		template<typename T>
			requires FusionStruct<T>
		constexpr size_t get_size(const T& val) {
			sizer s{};
			boost::fusion::for_each(val, [&](const auto& i) {
				s(i);
			});
			return s.size;
		}
	}
}