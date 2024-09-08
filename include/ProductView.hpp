#pragma once
#include <wx/panel.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/simplebook.h>
#include <wx/activityindicator.h>
#include <wx/busyinfo.h>
#include <wx/srchctrl.h>
#include <wx/infobar.h>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/progdlg.h>

#include <memory>
#include <stack>
#include <future>
#include <set>
#include <chrono>
#include <condition_variable>


#include "AuiTheme.hpp"
#include "DataModel.hpp"
#include "serialiser.h"
#include "Grape.hpp"
#include "Workspace.hpp"
#include "AddProduct.hpp"
#include "ProductInfo.hpp"

namespace ab {
	class ProductView : public wxPanel {
	public:
		//book pages
		enum {
			WAIT,
			EMPTY,
			SERVER_ERROR,
			VIEW,
			INFO,
		};

		enum {
			ID_DATA_VIEW = wxID_HIGHEST + 1,
			ID_BOOK,
			ID_TOP_TOOL,
			ID_BOTTOM_TOOL,
			ID_SEARCH,
			ID_FORMULARY,
			ID_IMPORT_FORMULARY,
			ID_EXPORT_FORMULARY,
			ID_ADD_PRODUCT,
			ID_CREATE_FORMULARY,
			ID_SEARH_TIMER,
			ID_SELECT,
		};

		enum {
			col_name = 3,
			col_class = 5,
			col_formulation = 6,
			col_unit_price = 11,
			col_cost_price = 12,
			col_package_size = 13,
			col_stock_count = 14,
			col_strength = 1111,
		};

		ProductView(wxWindow* parent, wxWindowID id, const wxPoint& position = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTAB_TRAVERSAL);
		virtual ~ProductView();

		void CreateBook();

		void CreateView();
		void CreateInventoryView();
		void CreatePanels();
		void CreateToolBar();
		void CreateBottomTool();
		void CreateProductInfo();

		void Load();
		void Clear();
		void LoadProducts(const grape::collection_type<grape::product>& products);
	

		std::set<boost::uuids::uuid> mSelections;

		//notification
		void OnWorkspaceNotification(ab::Workspace::notif notif,
			wxWindow* win);

		std::set<boost::uuids::uuid> mProductSelect;
	private:
		void OnBack(wxCommandEvent& evt);
		void OnForward(wxCommandEvent& evt);
		void OnUpdateArrows(wxUpdateUIEvent& evt);
		void OnAddProduct(wxCommandEvent& evt);
		void OnContextMenu(wxDataViewEvent& evt);
		void OnItemActivated(wxDataViewEvent& evt);
		void OnFormularyToolbar(wxAuiToolBarEvent& evt);
		void OnImportFormulary(wxCommandEvent& evt);
		void OnExportFormulary(wxCommandEvent& evt);
		void OnCreateFormulary(wxCommandEvent& evt);
		void OnSearch(wxCommandEvent& evt);
		void OnSearchCleared(wxCommandEvent& evt);
		void OnSearchTimeOut(wxTimerEvent& evt);
		void OnSelect(wxCommandEvent& evt);
		

		//grape functions 
		void GetProducts(size_t begin, size_t limit);
		void SearchProducts(std::string sstring);


		void SetupAuiTheme();
		void OnAuiThemeChange();


		wxAuiManager mManager;
		wxDataViewCtrl* mView = nullptr;
		wxAuiToolBar* mTopTool = nullptr;
		wxAuiToolBar* mBottomTool = nullptr;
		wxSimplebook* mBook = nullptr;
		wxSearchCtrl* mSearchBar = nullptr;
		wxAuiToolBarItem* mBack = nullptr;
		wxAuiToolBarItem* mForward = nullptr;
		wxInfoBar* mInfoBar = nullptr;

		//product info
		ab::ProductInfo* mProductInfo = nullptr;

		//empty
		wxPanel* mEmptyPanel = nullptr;
		wxStaticText* mEmptyText = nullptr;
		wxButton* mEmptyButton = nullptr;

		//no internet connection
		wxPanel* mNoConnectionPanel = nullptr;
		wxStaticText* mNoConnectionText = nullptr;
		wxButton* mNoConnectionButton = nullptr;

		wxAuiToolBarItem* mFormularyTool;
		//wait
		wxPanel* mWaitPanel = nullptr;
		wxActivityIndicator* mActivity = nullptr;
		std::future<void> mWaitProducts;

		std::unique_ptr<ab::DataModel<ab::pproduct>> mModel;
		std::unique_ptr<ab::DataModel<grape::inventory>> mInventoryModel;
		
		std::future<void> mWaitSearch;
		std::atomic<bool> mStillSearching = false;
		wxTimer mSearchTimer;

		std::stack<long> mPageStack;
		DECLARE_EVENT_TABLE()
	};

};