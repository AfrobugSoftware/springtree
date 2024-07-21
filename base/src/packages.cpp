#include "packages.h"
static constexpr auto const flags = boost::archive::no_header | boost::archive::no_tracking |boost::archive::no_xml_tag_checking;

pof::base::pack_t pof::base::packer::operator()() const {
	std::vector<char> compressed;

	boost::iostreams::filtering_ostream fos;
	fos.push(boost::iostreams::bzip2_compressor());
	fos.push(boost::iostreams::back_inserter(compressed));
	ar::binary_oarchive archive{ fos, flags };

	archive << m_data;
	return compressed;
}

void pof::base::unpacker::operator()(const pack_t& package)
{
	boost::iostreams::array_source as{ reinterpret_cast< const char*>(package.data()), package.size()};
	boost::iostreams::filtering_istream ifs;
	ifs.push(boost::iostreams::bzip2_decompressor());
	ifs.push(as);

	ar::binary_iarchive archive{ ifs, flags };
	
	archive >> m_data;
}
