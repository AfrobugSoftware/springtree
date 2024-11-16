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
#include <functional>

#include "AuiTheme.hpp"
#include "DataModel.hpp"
#include "SearchPopup.hpp"
namespace ab {
	class SupplierView : public wxPanel
	{
	public:
		std::function<void(void)> mOnBack;
		enum {
			ID_SUPPLIER_VIEW = wxID_HIGHEST + 200,
			ID_INVOICE_VIEW,
			ID_INVOICE_PRODUCT_VIEW,
			ID_TOOL,
			ID_INVOICE_TOOL,
			ID_INVOICE_PRODUCT_TOOL,
			ID_SEARCH,
			ID_ADD_SUPPLIER,
			ID_CREATE_INVOICE,
			ID_BACK,
			ID_SUPPLIER_BACK,
			ID_PRODUCT_SEARCH,
			ID_CHANGE_QUANTITY,
			ID_REMOVE_PRODUCT,

		};
		enum {
			SUPPLIER_VIEW,
			INVOICE_VIEW,
			INVOICE_PRODUCT_VIEW,
			SUPPLIER_EMPTY,
			INVOICE_EMPTY,
			INVOICE_PRODUCT_EMPTY,
			WAIT_PANEL,
			SERVER_ERROR,

		};


		SupplierView(wxWindow* win, wxWindowID id = wxID_ANY,
			const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize,
			long style = wxTAB_TRAVERSAL | wxNO_BORDER);
		virtual ~SupplierView() = default;

		void Suppliers();
		void UnLoad();

		bool GotoInvoice(boost::uuids::uuid invenid);
		constexpr void ResetPage() { page = SUPPLIER_VIEW; }
	private:
		void OnBack(wxCommandEvent& evt);
		void OnAddSupplier(wxCommandEvent& evt);
		void OnAddInvoice(wxCommandEvent& evt);
		void OnInvoiceContextMenu(wxDataViewEvent& evt);
		void OnOpenSupplier(wxDataViewEvent& evt);
		void OnOpenInvoice(wxDataViewEvent& evt);
		void OnInvoiceProductContextMenu(wxDataViewEvent& evt);
		void OnProductSearch(wxCommandEvent& evt);
		void OnQuantityChange(wxCommandEvent& evt);
		void OnRemoveProductInInvoice(wxCommandEvent& evt);

		void AddStock(const ab::pproduct& prod);
		void OnAuiThemeChange();
		void CreateToolBar();
		void CreatePanels();
		void CreateViews();

		void SwitchTool(int page);
		void LoadSuppliers(int start, int end);
		void LoadInvoice(boost::uuids::uuid suppid, int start, int end);
		void LoadInvoiceProducts(boost::uuids::uuid invoiceID);
		void DoRetry();
		void UpdateTotals();

		wxAuiToolBar* mTools;
		wxAuiToolBar* mInvoiceTools;
		wxAuiToolBar* mInvoiceProductTools;
		wxStaticText* mSupplierName;
		wxStaticText* mSupplierInvoiceName;
		wxAuiToolBarItem* mName1;
		wxAuiToolBarItem* mName2;
		wxSearchCtrl* mSupplierSearch;
		wxSearchCtrl* mInvoiceProductSearch;
		wxSimplebook* mBook;
		wxDataViewCtrl* mSupplierView;
		wxDataViewCtrl* mInvoiceView;
		wxDataViewCtrl* mInvoiceProductView;
		wxPanel* mCSPanel;
		wxStaticText* mTotalStock;
		wxStaticText* mTotalAmount;
		wxPanel* mEmpty;
		wxPanel* mEmptyInvoice;
		wxPanel* mEmptyInvoiceProduct;
		wxPanel* mWaitPanel;
		wxActivityIndicator* mWaitIndicator;
		wxPanel* mServerErrorPanel;
		wxStaticText* mServerErrorText;

		using supplier_t = ab::DataModel<grape::supplier>;
		using invoice_t  = ab::DataModel<boost::fusion::vector<
			boost::uuids::uuid,
			std::string,
			std::chrono::system_clock::time_point
		>>;
		using product_t = ab::DataModel<grape::invoice>;
		
		supplier_t* mSupplierModel;
		invoice_t*  mInvoiceModel;
		product_t*  mInvoiceProductModel;

		grape::supplier mCurSupp;
		invoice_t::fusion_t  mCurInvoice;

		ab::SearchPopup* mSearchPopup;
		int page = -1;

		wxAuiManager mManager;
		DECLARE_EVENT_TABLE();
	};

};