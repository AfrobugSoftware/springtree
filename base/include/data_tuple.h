#pragma once
#include "Data.h"
#include "errc.h"

#include <tuple>
#include <type_traits>

namespace pof
{
	namespace base {

		template<typename arg_t>
		void do_(pof::base::data::metadata_t& meta)
		{
			if constexpr (std::is_integral_v<arg_t>) {
				if constexpr (sizeof(arg_t) == 4) {
					if constexpr (std::is_signed_v<arg_t>) {
						meta.push_back(pof::base::data::kind::int32);
					}else {
						meta.push_back(pof::base::data::kind::uint32);
					}
				}
				else {
					if constexpr (std::is_signed_v<arg_t>) {
						meta.push_back(pof::base::data::kind::int64);
					}else{
						meta.push_back(pof::base::data::kind::uint64);
					}
				}
			}
			else if constexpr (std::is_floating_point_v<arg_t>) {
				if constexpr (sizeof(arg_t) == 4) {
					meta.push_back(pof::base::data::kind::float32);
				}
				else {
					meta.push_back(pof::base::data::kind::float64);
				}
			}
			else if constexpr (std::is_same_v<pof::base::data::blob_t, arg_t>) {
				meta.push_back(pof::base::data::kind::blob);
			}
			else if constexpr (std::is_same_v<pof::base::data::datetime_t, arg_t>) {
				meta.push_back(pof::base::data::kind::datetime);
			}
			else if constexpr (std::is_same_v<pof::base::data::text_t, arg_t>) {
				meta.push_back(pof::base::data::kind::text);
			}
			else if constexpr (std::is_same_v<pof::base::data::duuid_t, arg_t>) {
				meta.push_back(pof::base::data::kind::uuid);
			}
			else if constexpr (std::is_same_v<pof::base::data::currency_t, arg_t>) {
				meta.push_back(pof::base::data::kind::currency);
			}
			else {
				assert(false && "INVALID TYPE SPECIFIED");
			}
		}

		template<typename tuple_t, size_t i>
		struct loop
		{
			static void for_each(pof::base::data& d) {
				using arg_t = std::decay_t<std::tuple_element_t<i, tuple_t>>;
				auto& meta = d.get_metadata();
				loop<tuple_t, i - 1>::template for_each(d);
				do_<arg_t>(meta);
			}
		};

		template<typename tuple_t>
		struct loop<tuple_t, 0>
		{
			static void for_each(pof::base::data& d) {
				using arg_t = std::decay_t<std::tuple_element_t<0, tuple_t>>;
				auto& meta = d.get_metadata();
				do_<arg_t>(meta);
			}
		};

		//TODO:
		//add a concept to the list of args types, they must be the type of the 
		//selected data datatypes eg float double text_t blob etc..

		template<typename... args>
		void adapt(pof::base::data& d) {
			using bft = std::tuple<args...>;
			constexpr size_t len = sizeof...(args);
			loop<bft, len - 1>::template for_each(d);
		}
	};
};