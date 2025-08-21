#pragma once
#include <wx/panel.h>
#include <wx/dataview.h>
#include <wx/aui/auibar.h>
#include <wx/aui/framemanager.h>
#include <wx/simplebook.h>
#include <wx/dateevt.h>
#include <wx/datectrl.h>
#include <wx/activityindicator.h>
#include <wx/stattext.h>
#include "AuiTheme.hpp"

#include <functional>
#include <array>
#include <chrono>
#include <atomic>

namespace ab
{
	class Reports : public wxPanel
	{
	public:
		std::function<void()> mOnBack;

		enum {
			MAIN = 0,
			WAIT,
			EMPTY,
			REPORT_ERR,
			EOD,
			EOM,
			RANGE,
			PL,
			PURCHASE,
			MAX,
		};
		std::array<wxDataViewListCtrl*, MAX - 3> mReportViews;
		std::array<std::function<void()>, MAX - 3> mLoadReports;
		enum {
			ID_REPORT_LIST = wxID_HIGHEST + 300,
			ID_BACK,
			ID_DOWNLOAD_EXCEL,
			ID_START_DATE,
			ID_END_DATE,
			ID_DATE_TYPE,
		};
		Reports(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTAB_TRAVERSAL);
		virtual ~Reports() = default;

		void CreateToolbar();
		void CreateListView();
		void CreatePanels();
		void CreateReportViews();
		void SetupAuiTheme();
		void OnAuiThemeChange();

		void ShowName(int page);
		void UnLoad();
	private:
		void DoRetry();

		void OnListActivated(wxDataViewEvent& evt);
		void OnDownloadExcel(wxCommandEvent& evt);
		void OnBack(wxCommandEvent& evt);
		void OnDateChanged(wxDateEvent& evt);
		
		void LoadEOM();
		void LoadEOD();

		int page = MAIN;
		wxSimplebook* mBook;
		wxAuiManager mManager;
		wxDataViewListCtrl* mReportList;
		wxPanel* mEmpty;
		wxPanel* mWaitPanel;
		wxPanel* mServerErrorPanel;
		wxStaticText* mServerErrorText;
		wxStaticText* mReportName;
		wxAuiToolBarItem* mNameItem;
		wxActivityIndicator* mWaitIndicator;
		wxAuiToolBar* mTools;
		wxDatePickerCtrl* mStartDatePicker;
		wxDatePickerCtrl* mEndDatePicker;
		wxChoice* mDateTypeChoice;

		std::atomic<std::chrono::year_month_day> mStartDate;
		std::atomic<std::chrono::year_month_day> mEndDate;
		std::atomic_int mDateType; //0 - year, 1 - month, 2 - day
		DECLARE_EVENT_TABLE()
	};
};