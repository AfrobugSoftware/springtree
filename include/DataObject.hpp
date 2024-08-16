#pragma once
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include "serialiser.h"

#include <optional>
#include <utility>
#include <functional>
#include <algorithm>
#include <vector>

namespace ab {
	class DataObject : public wxDataObject {
	public:
		using data_t = std::vector<std::uint8_t>;

		DataObject(const std::string& format = "ab::dataobject"s) : mFormat(format) {}
		virtual ~DataObject() = default;

		virtual void GetAllFormats(wxDataFormat* formats, Direction dir) const override;
		virtual size_t GetDataSize(const wxDataFormat& format) const;
		virtual wxDataFormat GetPreferredFormat(Direction WXUNUSED(dir)) const override;

		virtual bool GetDataHere(const wxDataFormat& format, void* buffer) const override;
		virtual bool SetData(const wxDataFormat& format, size_t len, const void* buf) override;

		virtual size_t GetFormatCount(Direction dir) const override {
			//only supports the row_t format. 
			return 1;
		}

		template<grape::FusionStruct T>
		auto GetData() const -> std::optional<T> {
			if (!mFromData.has_value())  return std::nullopt;

			auto& v = mFromData.value();
			auto&& [t, buf] =
				grape::serial::read<T>(boost::asio::buffer(v));
			return t;
		}

		void SetData(data_t&& data) { mData = std::forward<data_t>(data); }
	private:
		wxDataFormat mFormat;
		std::optional<data_t> mData;
		std::optional<data_t> mFromData;
	};
};