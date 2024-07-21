#pragma once
#include "Data.h"

#include <vector>
#include <tuple>
#include <utility>
#include <type_traits>
#include <ranges>


namespace pof {
	namespace base {

		template<typename tuple_t, class T> struct index_of;

		template<typename U, typename T>
		struct index_of<std::tuple<U>, T> { enum { value = -1 }; };

		template<typename T>
		struct index_of<std::tuple<T>, T> { enum { value = 0 }; };

		template<typename T, typename...U>
		struct index_of<std::tuple<T, U...>, T> { enum { value = 0 }; };

		template<typename T, typename U, typename ...S>
		struct index_of<std::tuple<U, S...>, T>
		{
		private:
			enum { temp = index_of<std::tuple<S...>, T>::value };
		public:
			enum { value = temp == -1 ? -1 : 1 + temp };
		};

		template<typename T>
		class is_database_type
		{
			using special_types = std::tuple<pof::base::data::text_t, pof::base::data::blob_t, nullptr_t, pof::base::data::datetime_t, pof::base::data::duuid_t, pof::base::currency>;
		public:
			enum { value = (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T> || index_of<special_types, T>::value >= 0) };
		};

		template<typename... Args>
		class relation : public std::vector<std::tuple<Args...>>
		{
		public:
			constexpr const static size_t arg_size = sizeof...(Args);
			using tuple_t = std::tuple<Args...>;
			using base_t = std::vector<std::tuple<Args...>>;

			using iterator = base_t::iterator;
			using const_iterator = base_t::const_iterator;
		};
	};
};