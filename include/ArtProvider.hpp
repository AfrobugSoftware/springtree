#pragma once
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <map>

#include <wx/artprov.h>

#include "errc.h"

namespace fs = std::filesystem;
namespace ab {
	class ArtProvider : public wxArtProvider
	{
	public:
		using bitmap_map_t = std::unordered_map<std::string, wxBitmap>;
		ArtProvider();

	protected:
		void CreateArtStore();
		virtual wxSize DoGetSizeHint(const wxArtClient& client)  override;
		virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& clinet, const wxSize& size) override;
	private:
		std::unordered_map<std::string, bitmap_map_t> mArtMapByClient;
		std::unordered_map<std::string, wxBitmap> mArtMap;
	};
};