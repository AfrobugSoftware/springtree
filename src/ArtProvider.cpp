#include "ArtProvider.hpp"
#include "Application.hpp"


ab::ArtProvider::ArtProvider() {
	CreateArtStore();
}

void ab::ArtProvider::CreateArtStore()
{
	auto AssertPath = wxGetApp().mAsserts / "icons";
	try {
		for (auto& DirEntry : fs::directory_iterator(AssertPath))
		{
			static const std::map<std::string_view, wxBitmapType> Exts{ {".jpg", wxBITMAP_TYPE_JPEG}, {".png", wxBITMAP_TYPE_PNG},  {".ico", wxBITMAP_TYPE_ICO} };
			if (DirEntry.is_directory()) continue;
			const auto exts = DirEntry.path().filename().extension().string();
			auto iter = Exts.find(exts);
			if (iter != Exts.end()) {
				auto name = DirEntry.path().stem().string();
				mArtMap.insert({ std::move(name), wxBitmap(DirEntry.path().string(), iter->second) });
			}
		}
	}
	catch (std::filesystem::filesystem_error& error) {
		spdlog::critical(error.what());
	}
}

wxSize ab::ArtProvider::DoGetSizeHint(const wxArtClient& client)
{
	//should return a size for a particular clients
	return wxArtProvider::DoGetSizeHint(client);
}

wxBitmap ab::ArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& clinet, const wxSize& size)
{
	auto iter = mArtMap.find(id.ToStdString());
	if (iter != mArtMap.end()) {
		return iter->second;
	}
	return wxArtProvider::CreateBitmap(id, clinet, size);
}
