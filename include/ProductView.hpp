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


#include "AuiTheme.hpp"
#include "DataModel.hpp"
#include "serialiser.h"
#include "Grape.hpp"
#include "Workspace.hpp"
#include "AddProduct.hpp"


BOOST_FUSION_DEFINE_STRUCT(
	(ab), pproduct,
	(boost::uuids::uuid, id)
	(boost::uuids::uuid, formulary_id)
	(std::int64_t, serial_num)
	(std::string, name)
	(std::string, generic_name)
	(std::string, cls)
	(std::string, formulation)
	(std::string, strength)
	(std::string, strength_type)
	(std::string, usage_info)
	(std::string, indications)
	(pof::base::currency, unit_price)
	(pof::base::currency, cost_price)
	(std::int64_t, package_size)
	(std::int64_t, stock_count)
	(std::string, sideeffects)
	(std::string, barcode)
	(std::int64_t, category_id)
	(std::int64_t, min_stock_count)
)


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
			ID_IMPORT_FORULARY,
			ID_ADD_PRODUCT,
		};

		enum {
			col_name = 3,
			col_class = 5,
			col_formulation = 6,
			col_unit_price = 10,
			col_cost_price = 11,
			col_package_size = 12,
			col_stock_count = 13,
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

		void Load();
		void Clear();
		void LoadProducts(const grape::collection_type<grape::product>& products);
		void SwitchPage(long page);

		std::set<boost::uuids::uuid> mSelections;

		//notification
		void OnWorkspaceNotification(ab::Workspace::notif notif,
			wxWindow* win);
	private:
		void OnBack(wxCommandEvent& evt);
		void OnForward(wxCommandEvent& evt);
		void OnImportFormulary(wxCommandEvent& evt);
		void OnUpdateArrows(wxUpdateUIEvent& evt);
		void OnAddProduct(wxCommandEvent& evt);
		void OnContextMenu(wxDataViewEvent& evt);
		void OnItemActivated(wxDataViewEvent& evt);

	
		
		//grape functions 
		void GetProducts(size_t begin, size_t limit);
		


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

		//empty
		wxPanel* mEmptyPanel = nullptr;
		wxStaticText* mEmptyText = nullptr;
		wxButton* mEmptyButton = nullptr;

		//no internet connection
		wxPanel* mNoConnectionPanel = nullptr;
		wxStaticText* mNoConnectionText = nullptr;
		wxButton* mNoConnectionButton = nullptr;


		//wait
		wxPanel* mWaitPanel = nullptr;
		wxActivityIndicator* mActivity = nullptr;
		std::future<void> mWaitProducts;

		std::unique_ptr<ab::DataModel<ab::pproduct>> mModel;
		std::unique_ptr<ab::DataModel<grape::inventory>> mInventoryModel;
		
		std::stack<long> mPageStack;
		DECLARE_EVENT_TABLE()
	};

};