#pragma once
#include "Data.h"
#include <tuple>

namespace pof {
	namespace base {





		class visit {
		public:
			static bool build(const pof::base::data::row_t& row)
			{
				auto& v = row.first;
				for (auto iter = v.begin(); iter != v.end() - 1; iter++){
						
				}
			}


		};


	};
};