#pragma once
#include <wx/frame.h>
#include <wx/aui/aui.h>
#include <wx/simplebook.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <date/date.h>
#include <wx/aboutdlg.h>

#include <chrono>
#include <fmt/chrono.h>
#include <format>


#include "Workspace.hpp"
#include "Module.hpp"
#include "AuiTheme.hpp"

#include "ProductView.hpp"
#include "SaleView.hpp"

namespace ab {
	class MainFrame : public wxFrame
	{
	public:
		constexpr static const std::array<std::string_view, 12> monthNames = { "Jaunary", "Febuary", "March", "April",
		"May", "June", "July", "August", "September", "October", "November", "December" };

		constexpr static const std::array<std::string_view, 7> dayNames = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Sarturday" };
		enum {
			WELCOME = 0,
			WORKSPACE
		};

		enum {
			ID_MODULE = wxID_HIGHEST + 10,
			ID_WORKSPACE,
			ID_PAGER,
			ID_ABOUT,
		};


		MainFrame();
		MainFrame(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size);
		virtual ~MainFrame();

		void SetupAuiTheme();
		void CreateMenubar();
		void CreateToolbar();

		void CreateModules();
		void CreateWorkspace();
		void CreateWelcomePage();
		void CreateSelectList();
		void CreateImageList();
	private:
		void OnWelcomePageSelect(wxListEvent& evt);
		void OnAbout(wxCommandEvent& evt);
		void OnIdle(wxIdleEvent& evt);

		//signals
		void OnModuleActivated(const ab::mod& mod, ab::module_evt evt);
		void OnWorkspaceNotif(ab::Workspace::notif notif, wxWindow* win);
		void OnAuiThemeChange();

		wxStaticText* time1 = nullptr;
		wxStaticText* date1 = nullptr;
		wxStaticText* pharmName = nullptr;

		wxAuiManager mManager;
		wxAuiToolBar* mToolbar = nullptr;
		wxImageList* mImageList = nullptr;
		wxListCtrl* mSelectList = nullptr;
		wxSimplebook* mPager = nullptr;
		wxPanel* mWelcomePage = nullptr;
		ab::Workspace* mWorkspace = nullptr;
		ab::Modules* mModules = nullptr;

		ab::ProductView* mProductView = nullptr;
		ab::SaleView* mSaleView = nullptr;

		DECLARE_EVENT_TABLE()
	};
};