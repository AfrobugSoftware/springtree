#pragma once
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/datectrl.h>
#include <wx/dataview.h>
#include <wx/aui/auibar.h>
#include <wx/stattext.h>
#include <wx/srchctrl.h>
#include <wx/aui/framemanager.h>
#include <chrono>

#include "AuiTheme.hpp"
#include "DataModel.hpp"
#include "SearchPopup.hpp"
namespace ab {
	class SupplierView : public wxPanel
	{
	public:
		enum {
			ID_SUPPLIER_VIEW = wxID_HIGHEST + 200,
			ID_INVOICE_VIEW,
			ID_INVOICE_PRODUCT_VIEW,
			ID_TOOL,
			ID_SEARCH,
			ID_ADD_SUPPLIER,
			ID_CREATE_INVOICE,
			ID_BACK,
			ID_PRODUCT_SEARCH,

		};
		enum {
			SUPPLIER_VIEW,
			INVOICE_VIEW,
			SUPPLIER_EMPTY,
			INVOICE_EMPTY,
			SERVER_ERROR,

		};


		SupplierView(wxWindow* win, wxWindowID id = wxID_ANY,
			const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize,
			long style = wxTAB_TRAVERSAL | wxNO_BORDER);
		virtual ~SupplierView() = default;
	private:
		void OnAddSupplier(wxCommandEvent& evt);
		void OnAddInvoice(wxCommandEvent& evt);
		void OnInvoiceContextMenu(wxDataViewEvent& evt);

		void OnAddProduct(const ab::pproduct& product);

		void OnAuiThemeChange();
		void CreateToolBar();
		void CreatePanels();
		void CreateViews();
		
		wxAuiToolBar* mTools;
		wxAuiToolBar* mInvoiceTools;
		wxAuiToolBar* mInvoiceProductTools;
		wxStaticText* mSupplierName;
		wxSearchCtrl* mSupplierSearch;
		wxSearchCtrl* mInvoiceProductSearch;
		wxSimplebook* mBook;
		wxDataViewCtrl* mSupplierView;
		wxDataViewCtrl* mInvoiceView;
		wxDataViewCtrl* mInvoiceProductView;
		wxPanel* mCSPanel;
		wxStaticText* mTotalStock;
		wxStaticText* mTotalAmount;
		using supplier_t = ab::DataModel<boost::fusion::vector<
			std::string,
			std::chrono::system_clock::time_point,
			std::chrono::system_clock::time_point
		>>;
		using invoice_t = ab::DataModel<boost::fusion::vector<
			std::string,
			std::chrono::system_clock::time_point
		>>;
		using product_t = ab::DataModel<boost::fusion::vector<
		std::string,
		std::int64_t,
		pof::base::currency,
		std::chrono::system_clock::time_point,
		std::chrono::system_clock::time_point
		>>;
		
		supplier_t* mSupplierModel;
		invoice_t*  mInvoiceModel;
		product_t*  mInvoiceProductModel;

		ab::SearchPopup* mSearchPopup;
		wxAuiToolBarItem* mCreateInvoiceItem;
		wxAuiToolBarItem* mSupplierNameItem;
		int page = -1;

		wxAuiManager mManager;
		DECLARE_EVENT_TABLE();
	};

};