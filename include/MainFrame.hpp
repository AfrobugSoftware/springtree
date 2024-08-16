#pragma once
#include <wx/frame.h>
#include <wx/aui/aui.h>
#include <wx/simplebook.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <date/date.h>
#include <wx/aboutdlg.h>

#include "Workspace.hpp"
#include "Module.hpp"

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
		};


		MainFrame();
		MainFrame(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size);
		virtual ~MainFrame();

		void CreateMenubar();
		void CreateModules();
		void CreateWorkspace();
		void CreateeWelcomePage();
		void CreateSelectList();
	private:
		void OnWelcomePageSelect(wxListEvent& evt);
		void OnAbout(wxCommandEvent& evt);


		//signals
		void OnWorkspaceNotif(ab::Workspace::notif notif, size_t page);

		wxStaticText* time1 = nullptr;
		wxStaticText* date1 = nullptr;
		wxStaticText* pharmName = nullptr;

		wxListCtrl* mSelectList = nullptr;
		wxSimplebook* mPager = nullptr;
		wxPanel* mWelcomePage = nullptr;
		ab::Workspace* mWorkspace = nullptr;
		ab::Modules* mModules = nullptr;


		DECLARE_EVENT_TABLE()
	};
};