#pragma once
#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/valtext.h>
#include <wx/textctrl.h>
#include <wx/busyinfo.h>
#include <wx/aui/framemanager.h>
#include <wx/sysopt.h>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/propdlg.h>
#include <wx/snglinst.h>
#include <wx/debugrpt.h>
#include <wx/file.h>
#include <wx/ffile.h>
#include <wx/notifmsg.h>
#include <wx/splash.h>
#include <wx/fontdata.h>
#include <wx/dcmemory.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/activityindicator.h>


#include "serialiser.h"
#include "MainFrame.hpp"
#include "PharmacyManager.hpp"
#include "ArtProvider.hpp"

#include "net.h"
#include "netmanager.h"
#include "Grape.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <regex>
#include <boost/lexical_cast.hpp>


namespace js = nlohmann;
namespace fs = std::filesystem;
using namespace std::literals::string_literals;
namespace ab {

	class RegexValidator : public wxTextValidator
	{
	public:
		RegexValidator(std::regex&& reg, const std::string& errorstr = {});
		virtual wxObject* Clone() const override;
		virtual bool TransferFromWindow() override;
		virtual bool TransferToWindow() override;
		virtual bool Validate(wxWindow* parent) override;
		virtual wxString IsValid(const wxString& val) const override;

	private:
		std::string estr;
		std::regex pattern;

	};

	class Application : public wxApp {
	public:
		Application();
		virtual ~Application() {}

		virtual bool OnInit() override;
		virtual int OnExit() override;

		ab::MainFrame* mMainFrame = nullptr;
		fs::path mAsserts;

		void DecorateSplashScreen(wxBitmap& bmp);
		bool LoadSettings();
		bool SaveSettings();

		grape::app_details mAppDetails;
		grape::credentials mSessionCredentials;
		grape::address mAppAddress;


		ab::PharmacyManager mPharmacyManager;
		pof::base::net_manager mNetManager;
		std::string gVersion;

		//creation functions for test
		void CreateAddress();

		//send app ping to grape
		bool SendPing();
		wxTimer mPingTime;

		//panel helpers 
		std::tuple<wxPanel*, wxStaticText*, wxButton* > CreateEmptyPanel(wxWindow* parent, const std::string& text, const std::string& img = ""s, const std::string& client = wxART_OTHER);
		std::pair<wxPanel*, wxActivityIndicator*> CreateWaitPanel(wxWindow* parent, const std::string& text);


		//server helpers
		std::string ParseServerError(const grape::session::response_type& resp);

	private:
		bool LoadAppDetails();


	};

};

DECLARE_APP(ab::Application)