#pragma once
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>
#include <wx/splitter.h>
#include <wx/infobar.h>

#include "SearchPopup.hpp"
#include "Packs.hpp"

namespace ab {
	class SaleView : public wxPanel {
	public:
		enum {
			ID_CHECKOUT = wxID_HIGHEST + 1,
			ID_PRODUCT_SEARCH_NAME,
			ID_SAVE,
			ID_CLEAR,
			ID_PRODUCT_SCAN,
			ID_PACKS,
			ID_REMOVE_PRODUCT,
			ID_HIDE_PRODUCT_VIEW_PROPERTY,
			ID_PAYMENT_TYPE,
			ID_ACTIVE_UI_TEXT,
			ID_OPEN_SAVE_SALE,
			ID_REPRINT,
			ID_RETURN_SALE,
			ID_DISCOUNT,
			ID_NEW_SALE,
			ID_SALE_BOOK,

			//MUST BE THE LAST ID
			ID_SALE_VIEW,
			ID_PAY_VIEW,
		};

		enum {
			SALE_NOTEBOOK,
			SALE_EMPTY,
		};


		SaleView(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTAB_TRAVERSAL);
		virtual ~SaleView();

		void SetupAuiTheme();
		void OnAuiThemeChange();

		void CreateToolbar();
		void CreateView();
		void CreateMainPane();
		void PrintComplete(bool status, size_t work);
		ab::DataModel<grape::sale_display>* GetCurrentModel() const;
		grape::sale_receipt mReceipt;
	private:
		constexpr static size_t max_view = 10;
		pof::base::currency mCurTotal;
		void OnCheckOut(wxCommandEvent& evt);
		void OnClear(wxCommandEvent& evt);
		void OnSave(wxCommandEvent& evt);
		void OnNewSale(wxCommandEvent& evt);
		void OnOpenPacks(wxCommandEvent& evt);
		void OnBarcodeSearch(wxCommandEvent& evt);
		void OnProductSearch(wxCommandEvent& evt);
		void OnProductSearchCleared(wxCommandEvent& evt);
		void OnRemoveProduct(wxCommandEvent& evt);

		//sale book management
		void OnSaleNotebookClosed(wxAuiNotebookEvent& evt);
		void OnSaleNotebookClosing(wxAuiNotebookEvent& evt);
		void OnSaleNotebookChanged(wxAuiNotebookEvent& evt);

		void OnEditStarted(wxDataViewEvent& evt);
		void OnEditDone(wxDataViewEvent& evt);
		//signal
		void OnSearchedProduct(const grape::sale_display& saleproduct);
		void ClearTotals();
		void UpdateTotals();

		wxAuiManager mManager;
		wxAuiNotebook* mSaleNotebook; //have different sales be different pages in the book
		std::vector<wxDataViewCtrl*> mSaleView;
		std::vector<boost::uuids::uuid> mSaleIds;
		wxArrayString paymentTypes;
		wxPanel* mMainPane;
		wxAuiToolBar* mTopTools;
		wxAuiToolBar* mBottomTools;
		wxStaticText* mProductNameText;
		wxSearchCtrl* mProductNameValue;
		wxStaticText* mScanProduct;
		wxSearchCtrl* mScanProductValue;
		wxPanel* mProductViewPane = nullptr;
		wxPanel* mDataPane = nullptr;
		wxPropertyGrid* mPropertyManager = nullptr;
		wxDataViewCtrl* m_dataViewCtrl1 = nullptr;
		wxDataViewColumn* mSerialNumber = nullptr;
		wxDataViewColumn* mProductNameCol = nullptr;
		wxDataViewColumn* mQuantityColumn = nullptr;
		wxDataViewColumn* mExtPriceColumn = nullptr;
		wxDataViewColumn* mDiscountCol = nullptr;
		wxDataViewColumn* mPriceCol = nullptr;
		wxPanel* mSaleOutputPane;
		wxPanel* mSaleDisplayPane;
		wxPanel* mTextOutPut;
		wxPanel* mEmptyPanel;
		wxStaticText* mQuantity;
		wxStaticText* mQuantityValue;
		wxStaticText* mExtQuantity;
		wxStaticText* mExtQuantityItem;
		wxStaticText* mDiscountAmount;
		wxStaticText* mDiscountValue;
		wxStaticText* mTotalQuantity;
		wxStaticText* mTotalQuantityValue;
		wxStaticText* mTotalAmountLabel;
		wxStaticText* mTotalAmount;
		wxPanel* mSalePaymentButtonsPane;
		wxButton* mClear    = nullptr;
		wxButton* mSave     = nullptr;
		wxButton* mCheckout = nullptr;
		ab::SearchPopup* mSearchPopup = nullptr;
		std::string mCurPack;
		pof::base::data::duuid_t mCurPackID;
		wxInfoBar* mInfoBar = nullptr;
		wxStaticText* mActiveSaleId = nullptr;
		wxAuiToolBarItem* mActiveSaleTextItem = nullptr;
		wxAuiToolBarItem* mReprintItem = nullptr;
		wxAuiToolBarItem* mReturnItem = nullptr;

		//product properties
		wxStringProperty* productName = nullptr;
		wxStringProperty* genArray = nullptr;
		wxEditEnumProperty* dirArray = nullptr;
		wxIntProperty* stock = nullptr;
		wxStringProperty* strength = nullptr;
		wxStringProperty* strength_type = nullptr;
		wxEditEnumProperty* warning = nullptr;
		wxIntProperty* packageSize = nullptr;
		wxChoice* mPaymentTypes = nullptr;
		wxSimplebook* mBook = nullptr;
		wxPanel* mEmpty = nullptr;
		bool mLocked = false;
		size_t mSaleType = 0;


		DECLARE_EVENT_TABLE();
	};
};