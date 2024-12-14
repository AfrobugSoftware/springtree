#pragma once
#include <wx/panel.h>
#include <wx/dataview.h>
#include <wx/aui/auibar.h>
#include <wx/aui/framemanager.h>
#include <wx/simplebook.h>
#include "AuiTheme.hpp"

#include <functional>
#include <array>

namespace ab
{
	class Reports : public wxPanel
	{
	public:
		std::function<void()> mOnBack;
		std::array<wxDataViewCtrl, 4> mReportViews;
		enum {
			ID_REPORT_LIST = wxID_HIGHEST + 300,
			ID_BACK,
			ID_DOWNLOAD_EXCEL,
		};
		Reports(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTAB_TRAVERSAL);
		virtual ~Reports() = default;

		void CreateToolbar();
		void CreateListView();
		void CreatePanels();
		void CreateReportViews();

	private:
		void LoadReports();
		void DoRetry();

		void OnListActivated(wxDataViewEvent& evt);
		void OnDownloadExcel(wxCommandEvent& evt);
		void OnBack(wxCommandEvent& evt);

		wxSimplebook* mBook;
		wxAuiManager mManager;
		wxDataViewCtrl* mReportList;
		int page = wxNOT_FOUND;
		DECLARE_EVENT_TABLE()
	};
};