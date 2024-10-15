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
#include <wx/srchctrl.h>
#include <wx/infobar.h>
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
	private:
		void OnDataItemSelected(wxDataViewEvent& evt);
		bool CheckProduct(const ab::pproduct& product);
		
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
	private:
		constexpr static size_t max_view = 10;
		void OnCheckOut(wxCommandEvent& evt);
		void OnClear(wxCommandEvent& evt);
		void OnSave(wxCommandEvent& evt);
		void OnNewSale(wxCommandEvent& evt);
		void OnOpenPacks(wxCommandEvent& evt);

		void OnProductSearch(wxCommandEvent& evt);
		void OnProductSearchCleared(wxCommandEvent& evt);

		//sale book management
		void OnSaleNotebookClosed(wxAuiNotebookEvent& evt);

		//signal
		void OnSearchedProduct(const grape::sale_display& saleproduct);

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