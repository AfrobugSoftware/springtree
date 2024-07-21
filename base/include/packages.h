#pragma once
#include "Data.h"
#include <vector>


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <sstream>
#include <boost/serialization/vector.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>


namespace ar = boost::archive;
namespace pof
{
	namespace base {
		using pack_t = std::vector<char>;
		class packer {
		public:
			packer(const pof::base::data& data) : m_data(data) {}
			//pack_t operator()() const;
			pack_t operator()() const;
		private:
			const pof::base::data& m_data;
		};

		class unpacker
		{
		public:
			unpacker(pof::base::data& data) : m_data(data) {}
			void operator()(const pack_t& package);
		private:
			pof::base::data& m_data;
		};


	};
};