#pragma once
#include <wx/dialog.h>
#include <wx/aui/auibar.h>
#include <wx/aui/framemanager.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/simplebook.h>
#include <wx/dataview.h>
#include <wx/activityindicator.h>
#include <wx/srchctrl.h>

#include <boost/uuid/uuid.hpp>
#include "Auitheme.hpp"
#include "SearchPopup.hpp"

#include <optional>
namespace ab {
	class Packs : public wxDialog
	{
	public:
		enum {
			ID_TOOL = wxID_HIGHEST + 10,
			ID_TOOL_ADD_PACK,
			ID_SALE_PACK,
			ID_PACK_SELECT,
			ID_PACK_TOOL,
			ID_TOOL_GO_BACK,
			ID_TOOL_REMOVE_PRODUCT_PACK,
			ID_PACK_DATA,
			ID_OPEN_PACK,
			ID_RENAME_PACK,
			ID_REMOVE_PACK,
			ID_PRODUCT_SEARCH_NAME,
		};

		enum {
			PACK_EMPTY,
			PACK_PRODUCT_EMPTY,
			PACK_WAIT,
			PACK_SERVER_ERROR,
			PACK_VIEW,
			PACK_DATA,
		};

		Packs(wxWindow* parent, wxWindowID id = wxID_ANY, bool showSale = false,
			const std::string& title = "Pharmacy packs", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
		virtual ~Packs();
		std::optional<std::vector<grape::pharma_product>> mSalePackProducts;
	private:
		wxAuiManager mManager;
		wxAuiToolBar* mTopTools;
		wxAuiToolBar* mPackTools;
		wxSimplebook* mBook;
		wxPanel* mEmptyPack;
		wxPanel* mEmpty;
		wxPanel* mServerErrorPanel;
		wxStaticText* mServerErrorText;
		wxListCtrl* mPackSelect;
		wxDataViewCtrl* mPackData;
		wxDataViewColumn* mProductName;
		wxDataViewColumn* mProductQuantity;
		wxDataViewColumn* mPackageSize;
		wxDataViewColumn* mPrice;
		wxDataViewColumn* mExtPrice;
		wxPanel* m_panel4;
		wxStaticText* m_staticText1;
		wxStaticText* mTotalQuantity;
		wxStaticText* mTotalAmountText;
		wxStaticText* mTotalAmount;
		wxStaticText* mPackText = nullptr;
		wxAuiToolBarItem* mTextItem = nullptr;
		wxActivityIndicator* mWaitIndicator;
		wxPanel* mWaitPanel;
		wxSearchCtrl* mSearch;
		ab::SearchPopup* mSearchPopup;
		void SetupAuiTheme();
		void UpdateTheme();

		void CreatePanels();
		void CreateTopTools(); //naming is hard
		void CreatePackTools();
		void CreateSpeicalCols();
		void CreateView();
		void CreateSelectPanel();
		void ShowPack();

		void OnPackActivate(wxListEvent& evt);
		void OnEditPackName(wxListEvent& evt);
		void OnPackSelected(wxListEvent& evt);
		void OnAddPack(wxCommandEvent& evt);
		void OnRemovePack(wxCommandEvent& evt);
		void OnRemoveProductPack(wxCommandEvent& evt);
		void OnBack(wxCommandEvent& evt);
		void OnSalePack(wxCommandEvent& evt);
		void OnColEdited(wxDataViewEvent& evt);
		void OnRightClick(wxListEvent& evt);
		void OnRenamePack(wxCommandEvent& evt);
		void OnOpenPack(wxCommandEvent& evt);
		void OnSearch(wxCommandEvent& evt);

		void AddProduct(const ab::pproduct& pp);
		void SwitchTool();

		bool mShowSale = false;
		void UpdateTotals();
		void LoadPackDescSelect();
		void LoadPackModel(boost::uuids::uuid uuid);

		using packmodel_t = ab::DataModel<boost::fusion::vector<
			boost::uuids::uuid,
			std::string,
			std::int64_t,
			std::int64_t,
			pof::base::currency
			>>;
		packmodel_t* mPackModel;
		wxListItem mSelectedItem;
		DECLARE_EVENT_TABLE()
	};

};