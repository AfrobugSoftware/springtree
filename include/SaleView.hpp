#pragma once
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>
#include <wx/splitter.h>
#include <wx/dateevt.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/toolbar.h>
#include <wx/activityindicator.h>

#include <functional>
#include "Grape.hpp"
#include "DataModel.hpp"
#include "AuiTheme.hpp"

namespace ab {
	class SaleView : public wxPanel {
	public:
		enum {
			ID_CHECKOUT = wxID_HIGHEST + 1,
		};
		SaleView(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTAB_TRAVERSAL);
		virtual ~SaleView();

		void SetupAuiTheme();
		void OnAuiThemeChange();

		void CreateToolbar();
	private:
		void OnCheckOut(wxCommandEvent& evt);

		wxAuiManager mManager;
		wxAuiNotebook mSalesNotebook; //have different sales be different pages in the book

		DECLARE_EVENT_TABLE();
	};
};