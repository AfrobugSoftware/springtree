#include "DataObject.hpp"

void ab::DataObject::GetAllFormats(wxDataFormat* formats, Direction dir) const
{
	formats[0] = mFormat;
}

size_t ab::DataObject::GetDataSize(const wxDataFormat& format) const
{
	return mData.has_value() ?  mData.value().size() : (size_t)0;
}

wxDataFormat ab::DataObject::GetPreferredFormat(Direction WXUNUSED) const
{
	return mFormat;
}

bool ab::DataObject::GetDataHere(const wxDataFormat& format, void* buffer) const
{
	if ( mData.has_value() || mFormat.GetId() != format.GetId()) return false;
	auto& v = mData.value();

	std::copy(v.begin(), v.end(), reinterpret_cast<std::uint8_t*>(buffer));
	return true;
}

bool ab::DataObject::SetData(const wxDataFormat& format, size_t len, const void* buf)
{
	if (format.GetId() != mFormat.GetId()) return false;
	mFromData.emplace(len, 0x00);
	auto& v = mFromData.value();
	std::copy(reinterpret_cast<const uint8_t*>(buf), reinterpret_cast<const uint8_t*>(buf) + len, v.begin());
	return true;
}
