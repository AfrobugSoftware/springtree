#pragma once
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>
#include <wx/splitter.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/toolbar.h>

#include <functional>
#include "Grape.hpp"
#include "DataModel.hpp"
#include "AuiTheme.hpp"
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
			ID_PROPERTY_GRID = 8004,
			ID_SHOW_PRODUCT_SALE_HISTORY,
			ID_WARNINGS = 9000,
			ID_RESET,
			ID_ADD_BARCODE,
			ID_INVEN_MENU_CREATE_INVOICE,
			ID_INVEN_MENU_CHANGE_SUPPLIER_NAME,
			ID_SAVE_CHART_IMAGE,
			ID_SHOW_HIST_TABLE,
			ID_UPDATE_INVEN_STOCK,
			ID_INVEN_TOOLBAR,
			ID_INVEN_START_DATE_PICKER,
			ID_INVEN_STOP_DATE_PICKER
		};

		
		ProductInfo(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
		virtual ~ProductInfo() = default;
	
		void SetupAuitheme();
		void OnAuiThemeChange();

		void CreateMainTool();
		void CreateNotebook();
		void CreateInventoryView();
		void CreateHistoryView();
		void CreateProperyGrid();
		void CreateWarnings();


		void OnAddBarcode(wxCommandEvent& evt);
		void OnPropertyChanging(wxPropertyGridEvent& evt);
		//void OnBack(wxCommandEvent& evt);
		void OnHistory(wxCommandEvent& evt);
		void OnProductProperty(wxCommandEvent& evt);
		void OnAddStock(wxCommandEvent& evt);
		void OnAddBarcode(wxCommandEvent& evt);
		void OnCacheHint(wxDataViewEvent& evt);
		void OnDateChange(wxDateEvent& evt);

		void GetInventory(size_t begin, size_t limit);
		void GetHistory(size_t begin, size_t limit);
		void GetProductFormulary();

		wxAuiManager mManager;
		wxAuiNotebook* mNotebook;
		wxAuiToolBar* mMainToolbar;


		double mStubPrice;
		ab::pproduct mSelectedProduct;

		wxSimplebook* mInventoryBook;
		wxPanel* mInventoryPanel;
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
		
		wxPropertyGridManager* mProductInfoGridManager;
		wxPropertyGridPage* mProductInfoPage;
		wxPGProperty* m_propertyGridItem1;
		wxPGProperty* mNameItem;
		wxPGProperty* mGenericNameItem;
		wxPGProperty* mPackageSizeItem;
		wxPGProperty* mProductClass;
		wxPGProperty* mFormulationItem;
		wxPGProperty* mStrengthValueItem;
		wxPGProperty* mStrengthTypeItem;
		wxPGProperty* mMoreProductInfo;
		wxPGProperty* mDirForUse;
		wxPGProperty* mHealthCond;
		wxPGProperty* mProductDescription;
		wxPGProperty* mSideEffects;
		wxPGProperty* mSettings;
		wxPGProperty* mMinStockCount;
		wxPGProperty* mExpDateCount;
		wxPGProperty* mExpDatePeriod;
		wxPGProperty* mSaleSettings;
		wxPGProperty* mUnitPrice;
		wxPGProperty* mCostPrice;
		wxPGProperty* mBarcode;
		wxPGProperty* mCurStock;
		wxDatePickerCtrl* mInventoryDate;
		wxPGChoices ProductClassChoices;
		wxPGChoices FormulationChoices;
		wxPGChoices ExpChoices;
		wxPGChoices StrengthChoices;
		
		wxPanel* mEmpty = nullptr;



		DECLARE_EVENT_TABLE();
	};
};