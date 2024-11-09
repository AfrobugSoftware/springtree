#pragma once
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/dateevt.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/toolbar.h>
#include <wx/activityindicator.h>
#include <wx/srchctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/artprov.h>
#include <wx/stattext.h>
#include <wx/dcclient.h>
#include <wx/popupwin.h>

#include <functional>
#include "Grape.hpp"
#include "DataModel.hpp"
#include "AuiTheme.hpp"

#include <boost/signals2/signal.hpp>

namespace ab {
	class SearchPopup : public wxPopupTransientWindow {
	public:
		boost::signals2::signal<void(const grape::sale_display&)> sSelectedSignal;
		boost::signals2::signal<void(const ab::pproduct&)> sProductSignal;
		enum {
			ID_DATA_VIEW = 10,
		};

		enum {
			DATA_VIEW = 0,
			NO_RESULT,
			WAIT,
			ERROR_PANE,
		};

		SearchPopup(wxWindow* parent);
		virtual ~SearchPopup() = default;

		void ChangeFont(const wxFont& font);

		wxDataViewItem GetSelected() const { return mTable->GetSelection(); }
		void SetNext(bool forward = true);
		void SetActivated();

		size_t GetItemCount() const { return mTableModel->size(); }
		void Search(const std::string& str);
		void SearchProducts(std::string&& sstring);
		bool CheckProduct(const ab::pproduct& product);
	private:
		void SetupAuiTheme();
		void OnAuiThemeChange();

		void OnDataItemSelected(wxDataViewEvent& evt);

		wxPanel* mWaitPanel;
		wxActivityIndicator* mActivity;

		wxPanel* mErrorPanel;
		wxStaticText* mErrorText;
		wxButton* retry;


		std::atomic_bool mSearching;
		std::string mSearchString; //for retires
		wxAuiManager mPopManager;
		wxSimplebook* mBook = nullptr;
		wxPanel* mNoResult = nullptr;
		wxButton* mNoResultRetry = nullptr;
		wxStaticText* mNoResultText = nullptr;
		wxDataViewCtrl* mTable = nullptr;
		ab::DataModel<ab::pproduct>* mTableModel = nullptr;
		DECLARE_EVENT_TABLE()

	};

};