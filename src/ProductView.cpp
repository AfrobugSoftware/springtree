#include "ProductView.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::ProductView, wxPanel)
	EVT_TOOL(wxID_BACKWARD, ab::ProductView::OnBack)
	EVT_TOOL(wxID_FORWARD, ab::ProductView::OnForward)
	EVT_UPDATE_UI(wxID_FORWARD, ab::ProductView::OnUpdateArrows)
	EVT_UPDATE_UI(wxID_BACKWARD, ab::ProductView::OnUpdateArrows)
	EVT_TOOL(ab::ProductView::ID_ADD_PRODUCT, ab::ProductView::OnAddProduct)

	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ab::ProductView::ID_DATA_VIEW, ab::ProductView::OnContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::ProductView::ID_DATA_VIEW, ab::ProductView::OnItemActivated)
END_EVENT_TABLE()


ab::ProductView::ProductView(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	:wxPanel(parent, id, position, size, style), mManager(this, ab::AuiTheme::AUIMGRSTYLE)
{
	CreateBook();
	CreateToolBar();
	CreateBottomTool();


	CreateView();
	mManager.Update();
}

ab::ProductView::~ProductView()
{
}

void ab::ProductView::CreateBook()
{
	mBook = new wxSimplebook(this, ID_BOOK);
	mManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").CentrePane().Show());
}

void ab::ProductView::CreateView()
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	mInfoBar = new wxInfoBar(panel, wxID_ANY);
	mInfoBar->SetAutoLayout(true);

	mView = new wxDataViewCtrl(panel, ID_DATA_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mView->AssociateModel(mModel.get());

	mView->AppendTextColumn(wxT("Name"), col_name, wxDATAVIEW_CELL_INERT, FromDIP(450), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mView->AppendTextColumn(wxT("Strength"), col_strength, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mView->AppendTextColumn(wxT("Class"), col_class, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mView->AppendTextColumn(wxT("Formulation"), col_formulation, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mView->AppendTextColumn(wxT("Package Size"), col_package_size, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mView->AppendTextColumn(wxT("Stock Count"), col_stock_count, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mView->AppendTextColumn(wxT("Unit Price"), col_unit_price, wxDATAVIEW_CELL_INERT, FromDIP(70), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);

	sizer->Add(mInfoBar, wxSizerFlags().Expand().Border(wxALL, FromDIP(0)));
	sizer->Add(mView, wxSizerFlags().Expand().Proportion(1).Border(wxALL, FromDIP(0)));

	panel->SetSizer(sizer);
	panel->Layout();
	mBook->AddPage(panel, "Dataview", false);
}

void ab::ProductView::CreateInventoryView()
{
}

void ab::ProductView::CreatePanels()
{
	auto& app = wxGetApp();
	std::tie(mWaitPanel, mActivity) = app.CreateWaitPanel(mBook, "Please wait..");
	std::tie(mEmptyPanel, mEmptyText, mEmptyButton) = app.CreateEmptyPanel(mBook, "No products in store", "supplement-bottle");
	std::tie(mNoConnectionPanel, mNoConnectionText, mNoConnectionButton) = app.CreateEmptyPanel(mBook, "No connection", wxART_ERROR, wxSize(48,48), wxART_MESSAGE_BOX);
	//bind callbacks
	mEmptyButton->SetLabel("Add product");
	mEmptyButton->Bind(wxEVT_BUTTON, std::bind_front(&ab::ProductView::OnAddProduct, this));

	mNoConnectionButton->SetLabel("Retry");
	mNoConnectionButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		//try to load again
		auto& top = mPageStack.top();
		switch (top)
		{
		case VIEW:
			break;
		case INFO:
			break;
		default:
			break;
		}
	});

	mBook->AddPage(mWaitPanel,"Wiat", true);
	mBook->AddPage(mEmptyPanel, "Empty", false);
}

void ab::ProductView::CreateToolBar()
{
	mTopTool = new wxAuiToolBar(this, ID_TOP_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mTopTool->SetToolBitmapSize(wxSize(FromDIP(16), FromDIP(16)));

	mBack = mTopTool->AddTool(wxID_BACKWARD,wxEmptyString, wxArtProvider::GetBitmap("back"), "Back");
	mTopTool->AddSpacer(FromDIP(5));
	mForward = mTopTool->AddTool(wxID_FORWARD,wxEmptyString, wxArtProvider::GetBitmap("forward"), "Back");

	mSearchBar = new wxSearchCtrl(mTopTool, ID_SEARCH, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(450), FromDIP(-1)), wxWANTS_CHARS);
#ifndef __WXMAC__
	mSearchBar->ShowSearchButton(true);
#endif
	mSearchBar->ShowCancelButton(true);
	mSearchBar->SetDescriptiveText("Search for products");

	mTopTool->AddControl(mSearchBar);
	mTopTool->AddStretchSpacer();
	mTopTool->AddTool(ID_ADD_PRODUCT, "Add Product", wxArtProvider::GetBitmap("add", wxART_OTHER,FromDIP(wxSize(16,16))));
	mTopTool->Realize();
	mManager.AddPane(mTopTool, wxAuiPaneInfo().Name("TopToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(2).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductView::CreateBottomTool()
{
	mBottomTool = new wxAuiToolBar(this, ID_TOP_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mBottomTool->SetToolBitmapSize(wxSize(FromDIP(16), FromDIP(16)));

	mBottomTool->AddTool(ID_IMPORT_FORULARY, "Import formulary", wxArtProvider::GetBitmap("edit_note"));

	mBottomTool->Realize();
	mManager.AddPane(mBottomTool, wxAuiPaneInfo().Name("TopToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(2).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductView::Load()
{
	mBook->SetSelection(WAIT);
	mActivity->Start();
	mWaitProducts = std::async(std::launch::async,std::bind_front(&ab::ProductView::GetProducts, this), 0, 1000);
}

void ab::ProductView::OnBack(wxCommandEvent& evt)
{

}

void ab::ProductView::OnForward(wxCommandEvent& evt)
{
}

void ab::ProductView::OnImportFormulary(wxCommandEvent& evt)
{
}

void ab::ProductView::OnUpdateArrows(wxUpdateUIEvent& evt)
{
	wxWindowID id = evt.GetId();
	switch (id)
	{
	case wxID_BACKWARD:
	{
		//
		if (mPageStack.size() < 2) {
			mBack->GetWindow()->Disable();
		}
		else {
			mBack->GetWindow()->Enable();
		}
	}
	break;

	default:
		break;
	}
}

void ab::ProductView::OnAddProduct(wxCommandEvent& evt)
{
}

void ab::ProductView::OnContextMenu(wxDataViewEvent& evt)
{
	auto item = evt.GetItem();
	if (!item.IsOk()) return;
	wxMenu* menu = new wxMenu;


	mView->PopupMenu(menu);
}

void ab::ProductView::OnItemActivated(wxDataViewEvent& evt)
{
}

void ab::ProductView::GetProducts(size_t begin, size_t limit)
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
		grape::credentials cred;
		cred.pharm_id = app.mPharmacyManager.pharmacy.id;
		cred.branch_id = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();

		grape::page page;
		page.begin = begin;
		page.limit = limit;

		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(page);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		grape::serial::write(buf, page);

		auto fut = sess->req(http::verb::get, "/products/get"s, std::move(body));
		auto resp = fut.get();
		if (resp.result() == http::status::not_found) {
			mBook->SetSelection(EMPTY);
			return;
		}else if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& b = resp.body();
		if (b.empty()) throw std::logic_error("no data receievd");
		auto&& [col, buf2] = grape::serial::read<grape::collection_type<ab::pproduct>>(boost::asio::buffer(b));
		
		mModel->Reload(boost::fusion::at_c<0>(col));

		mActivity->Stop();
		mBook->SetSelection(VIEW);
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		mBook->SetSelection(GRAPE_EMPTY);
	}
}
