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
#include <wx/spinctrl.h>

#include <boost/asio/thread_pool.hpp>

#include <functional>
#include "Grape.hpp"
#include "DataModel.hpp"
#include "AuiTheme.hpp"
#include "Validator.hpp"
namespace ab {
	class ProductInfo : public wxPanel {
	public:
		enum {
			ID_TOOL_GO_BACK = wxID_HIGHEST + 2000,
			ID_DATA_VIEW,
			ID_DATA_VIEW_HIST,
			ID_MAINTOOL,
			ID_NOTEBOOK,
			ID_TOOL_ADD_INVENTORY,
			ID_TOOL_SHOW_PRODUCT_INFO,
			ID_PROPERTY_GRID,
			ID_SHOW_PRODUCT_SALE_HISTORY,
			ID_WARNINGS,
			ID_RESET,
			ID_ADD_BARCODE,
			ID_INVEN_MENU_CREATE_INVOICE,
			ID_INVEN_MENU_CHANGE_SUPPLIER_NAME,
			ID_SAVE_CHART_IMAGE,
			ID_SHOW_HIST_TABLE,
			ID_UPDATE_INVEN_STOCK,
			ID_INVEN_TOOLBAR,
			ID_INVEN_START_DATE_PICKER,
			ID_INVEN_STOP_DATE_PICKER,
		};

		//inventory book pages 
		enum {
			INVEN_VIEW,
			INVEN_WAIT,
			INVEN_EMPTY,
			INVEN_ERROR,
		};
		
		//history book pages
		enum {
			HIST_VIEW,
			HIST_WAIT,
			HIST_EMPTY,
			HIST_ERROR,
		};

		ProductInfo(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
		virtual ~ProductInfo();
	
		void SetupAuitheme();
		void OnAuiThemeChange();

		void CreateMainTool();
		void CreateInventoryView();
		void CreateHistoryView();
		void CreateProperyGrid();
		void CreateWarnings();
		void CreateChoices();

		void OnDateChange(wxDateEvent& evt);
		void OnAddBarcode(wxCommandEvent& evt);
		void OnPropertyChanging(wxPropertyGridEvent& evt);
		void OnPropertyChanged(wxPropertyGridEvent& evt);
		void OnBack(wxCommandEvent& evt);
		void OnHistory(wxCommandEvent& evt);
		void OnProductProperty(wxCommandEvent& evt);
		void OnAddStock(wxCommandEvent& evt);
		void OnCacheHint(wxDataViewEvent& evt);
		void OnSave(wxCommandEvent& evt);

		void EnableByFormulary();
		void GetInventory(size_t begin, size_t limit);
		void GetHistory(size_t begin, size_t limit);
		void GetProductFormulary();
		void Load();
		void UnLoad();

		wxAuiManager mManager;
		wxAuiToolBar* mMainToolbar;


		double mStubPrice;
		ab::pproduct mSelectedProduct;
		std::bitset<boost::mpl::size<grape::product>::value> mUpdateSet;
		std::bitset<boost::mpl::size<grape::pharma_product>::value> mUpdatePharmaSet;

		grape::formulary mProductFormulary;

		wxSimplebook* mInventoryBook;
		wxPanel* mInventoryPanel;
		wxPanel* mInventoryWaitPanel;
		wxPanel* mInventoryEmptyPanel;
		wxPanel* mInventoryErrorPanel;
		wxStaticText* mInventoryErrorText;
		wxActivityIndicator* mInventoryActivity;


		wxDataViewCtrl* mInventoryView;
		wxAuiToolBar* mInventoryToolbar;
		wxDatePickerCtrl* mStartDatePicker;
		wxDatePickerCtrl* mStopDatePicker;
		std::unique_ptr<ab::DataModel<grape::inventory>> mInventoryModel;

		wxAuiToolBar* m_auiToolBar1;
		wxAuiToolBarItem* m_tool1;
		wxAuiToolBarItem* mProductHist;
		wxAuiToolBarItem* mProductNameText;
		wxAuiToolBarItem* mShowAddInfo;

		wxSimplebook* mHistBook;
		wxPanel* mHistPanel;
		wxDataViewCtrl* mHistView;
		wxDataViewColumn* mInputDate;
		wxDataViewColumn* mBactchNo;
		wxDataViewColumn* mExpiryDate;
		wxDataViewColumn* mStockCount;
		wxDataViewColumn* mManuFactureName;
		std::unique_ptr<ab::DataModel<grape::sale_history>> mHistModel;

		wxPropertyGridManager* mProductInfoGridManager;
		wxPropertyGridPage* mProductInfoPage;
	
		std::array<wxPGProperty*, 14> p;
		wxPGChoices ProductClassChoices;
		wxPGChoices FormulationChoices;
		wxPGChoices StrengthChoices;
		
		wxStaticText* mNameLabel = nullptr;
		wxAuiToolBarItem* mNameLabelItem = nullptr;
		//wait futures
		std::atomic<bool> mInvenoryRunning;
		std::atomic<size_t> mInventoryCount;
		std::function<void(void)> mOnBack;
		DECLARE_EVENT_TABLE();
	};
};