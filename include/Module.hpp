#pragma once

#include <wx/panel.h>
#include <wx/treectrl.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include <boost/signals2/signal.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_set.hpp>

#include <list>
#include <functional>
#include "Grape.hpp"
#include "DataObject.hpp"

//enums
namespace ab {
	enum class module_evt : std::uint32_t {
		activated,
		collapsed,
		sel_changed,
		drag_begin,
		drag_end,
		right_click,
	};
};

namespace ab {
	//allow mod to be hashable
	struct mod {
		wxTreeItemId id;
		std::string name;
		int img = -1;
		wxWindow* win = NULL;
		std::vector<ab::mod> children;
		std::function<void(const ab::mod&, ab::module_evt)> callback;
	};


	//extern bool operator==(const wxTreeItemId& a, const wxTreeItemId& b);
	extern std::size_t hash_value(wxTreeItemId const& b);

	class Modules : public wxPanel
	{
	public:
		enum {
			ID_TREE = wxID_HIGHEST + 10,
		};

		boost::signals2::signal<void(const wxTreeItemId&, const std::string&, const std::string&)> mLabelChange;

		Modules(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(248, 680), long style = wxNO_BORDER | wxTAB_TRAVERSAL);
		virtual ~Modules() = default;
		
		void CreateTree();

		void OnTreeActivated(wxTreeEvent& evt);
		void OnRightClick(wxTreeEvent& evt);
		void OnDragBegin(wxTreeEvent& evt);
		void OnDragEnd(wxTreeEvent& evt);
		void OnBeginEditLabel(wxTreeEvent& evt);
		void OnEndEditLabel(wxTreeEvent& evt);

		inline void SetImageList(wxImageList* imglist) { mModuleTree->SetImageList(imglist); }
		void ReloadAccountDetails();

		void UpdatesLogo(const wxBitmap& bm);
		void ActivateModule(const wxTreeItemId& id);


		void Add(ab::mod&& mod);
		wxTreeItemId AddChild(const wxTreeItemId& parent, ab::mod&& mod);
		bool RemoveChild(const wxTreeItemId& parent, const ab::mod& mod);
	private:
		friend class MainFrame;

		wxPanel* m_panel1;
		wxStaticBitmap* m_bitmap1;
		wxPanel* m_panel2;
		wxTreeCtrl* mModuleTree;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText1;
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText4;
	

		wxTreeItemId mPharmacy;
		wxTreeItemId mTransactions;

		wxTreeItemId mPrescriptions;
		wxTreeItemId mPaitents;
		wxTreeItemId mPoisionBook;
		wxTreeItemId mProducts;
		wxTreeItemId mSales;
		wxTreeItemId mOrders;
		wxTreeItemId mRequisitions;
		wxTreeItemId mAuditTrails;

		std::vector<ab::mod> mMods;
		bool Search(std::vector<ab::mod>& hey, const wxTreeItemId& needle, ab::mod** found);


		DECLARE_EVENT_TABLE()

	};
};