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
	SetupAuiTheme();
	CreateBook();
	CreatePanels();
	CreateToolBar();
	CreateBottomTool();


	CreateView();
	mManager.Update();
}

ab::ProductView::~ProductView()
{
	mManager.UnInit();
	mModel.release();
}

void ab::ProductView::CreateBook()
{
	mBook = new wxSimplebook(this, ID_BOOK);
	mManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").CentrePane().Show());
}

void ab::ProductView::CreateView()
{
	wxPanel* panel = new wxPanel(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	mInfoBar = new wxInfoBar(panel, wxID_ANY);
	mInfoBar->SetAutoLayout(true);

	mView = new wxDataViewCtrl(panel, ID_DATA_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mModel = std::make_unique<ab::DataModel<ab::pproduct>>();
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
		
	});

	mBook->AddPage(mWaitPanel,"Wait", true);
	mBook->AddPage(mEmptyPanel, "Empty", false);
	mBook->AddPage(mNoConnectionPanel, "Server error", false);
}

void ab::ProductView::CreateToolBar()
{
	mTopTool = new wxAuiToolBar(this, ID_TOP_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mTopTool->SetToolBitmapSize(wxSize(FromDIP(16), FromDIP(16)));

	mBack = mTopTool->AddTool(wxID_BACKWARD,wxEmptyString, wxArtProvider::GetBitmap("back", wxART_OTHER, wxSize(16,16)), "Back");
	mTopTool->AddSpacer(FromDIP(5));
	mForward = mTopTool->AddTool(wxID_FORWARD,wxEmptyString, wxArtProvider::GetBitmap("forward", wxART_OTHER, wxSize(16,16)), "forward");

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
	mManager.AddPane(mTopTool, wxAuiPaneInfo().Name("TopToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductView::CreateBottomTool()
{
	mBottomTool = new wxAuiToolBar(this, ID_BOTTOM_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mBottomTool->SetToolBitmapSize(wxSize(FromDIP(16), FromDIP(16)));

	mBottomTool->AddTool(ID_IMPORT_FORULARY, "Import formulary", wxArtProvider::GetBitmap("edit_not", wxART_OTHER, wxSize(16,16)));

	mBottomTool->Realize();
	mManager.AddPane(mBottomTool, wxAuiPaneInfo().Name("BottomToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(2).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductView::Load()
{
	mBook->SetSelection(WAIT);
	mActivity->Start();
	mWaitProducts = std::async(std::launch::async,std::bind_front(&ab::ProductView::GetProducts, this), 0, 1000);
}

void ab::ProductView::Clear()
{
	mModel->Clear();
}

void ab::ProductView::LoadProducts(const grape::collection_type<grape::product>& products)
{
}

void ab::ProductView::OnBack(wxCommandEvent& evt)
{

}

void ab::ProductView::OnForward(wxCommandEvent& evt)
{
}

void ab::ProductView::OnImportFormulary(wxCommandEvent& evt)
{
	auto& app = wxGetApp();
	auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
	grape::session::response_type resp{};
	std::string target = std::format("/product/formulary/checkname/{}", app.mPharmacyManager.pharmacy.name);
	auto fut = sess->req(http::verb::get, target, {});
	grape::credentials cred;
	cred.pharm_id = app.mPharmacyManager.pharmacy.id;
	cred.branch_id = app.mPharmacyManager.branch.id;
	cred.account_id = app.mPharmacyManager.account.account_id;
	cred.session_id = app.mPharmacyManager.account.session_id.value();
	constexpr const size_t size = grape::serial::get_size(cred);

	{
		wxBusyInfo wait("Checking for formulary\nPlease wait...");
		resp = fut.get();
	}
	if (resp.result() != http::status::not_found) {
		bool load = wxMessageBox("This pharmacy already has a formulary\nDo you want load it?","Formulary", wxICON_INFORMATION | wxYES_NO) == wxYES;
		if (!load) return;

		//Load in formulary
		grape::body_type body(size, 0x00);
		grape::serial::write(boost::asio::buffer(body), cred);
		fut = sess->req(http::verb::get, "/product/formulary/getproducts", std::move(body));

		{
			wxBusyInfo wait("Loading products from formulary\nPlease wait...");
			resp = fut.get();
		}
		
		return;
	}


	wxFileDialog fileDialog(this, "Formulary Import", wxEmptyString, wxEmptyString, "form files (*.form)|*.form",wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (fileDialog.ShowModal() == wxID_CANCEL)
		return;
	auto filename = fs::path(fileDialog.GetPath().ToStdString());
	if (filename.extension().string() != ".form") {
		wxMessageBox(fmt::format("{} is not a formulary file", filename.string()), "Formulary", wxICON_WARNING | wxOK);
		return;
	}

	std::ifstream file(filename);
	if (!file.is_open()) {
		wxMessageBox(fmt::format("Cannot open {} permission denied", filename.string()), "Formulary", wxICON_ERROR | wxOK);
		return;
	}
	std::stringstream data;
	data << file.rdbuf();
	try {
		const js::json form = js::json::parse(data.str());
		const js::json header = form["header"];
		const js::json products = form["products"];
		bool has_inven = static_cast<bool>(form["has_inventory"]);

		const size_t count = static_cast<std::uint64_t>(header["product_count"]);
		std::chrono::system_clock::time_point p(std::chrono::system_clock::duration(static_cast<std::uint64_t>(header["timestamp"])));

		wxMessageBox(fmt::format("Importing formulary\nName: {}\n\nAddress: {}\n\nDate created: {:%d:%m:%Y}\n\nProducts count: {:d}",
			static_cast<std::string>(header["pharmacy"]),
			static_cast<std::string>(header["address"]),
			p,
			count), "Formulary import", wxICON_INFORMATION | wxOK);
		wxProgressDialog dlg("Loading formulary", "please wait...", 100, this, wxPD_CAN_ABORT | wxPD_SMOOTH | wxPD_APP_MODAL | wxPD_AUTO_HIDE);

		//create the formulary ?



		std::vector<grape::product> prods;
		prods.reserve(products.size());
		for (const auto& prod : products) {
			auto& p         = prods.emplace_back(grape::product{});
			p.id            = boost::uuids::nil_uuid();
			p.serial_num    = 0;
			p.name          = static_cast<std::string>(prod["name"]);
			p.generic_name  = static_cast<std::string>(prod["generic_name"]);
			p.class_        = static_cast<std::string>(prod["class"]);
			p.formulation   = static_cast<std::string>(prod["formulation"]);
			p.strength      = static_cast<std::string>(prod["strength"]);
			p.strength_type = static_cast<std::string>(prod["strength_type"]);
			p.usage_info    = static_cast<std::string>(prod["usage_info"]);
			p.description   = static_cast<std::string>(prod["description"]);
			p.indications   = static_cast<std::string>(prod["health_conditions"]);
			p.sideeffects   = static_cast<std::string>(prod["side_effects"]);
		}

	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Formulary error:\n{}", exp.what()), "Formulary", wxICON_ERROR | wxOK);
		spdlog::error(exp.what());
		return;
	}
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
			mBack->SetActive(false);
		}
		else {
			mBack->SetActive(true);
		}
	}
	break;

	default:
		break;
	}
}

void ab::ProductView::OnAddProduct(wxCommandEvent& evt)
{
	ab::AddProductDialog ap(nullptr);
	wxWindowID ret = ap.ShowModal();
	if (ret == wxID_OK) {
		Load();
	}
	//handle abourt
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

void ab::ProductView::OnWorkspaceNotification(ab::Workspace::notif notif, wxWindow* win)
{
	if (this != win) return;

	switch (notif)
	{
	case ab::Workspace::notif::closed:
	case ab::Workspace::notif::deleted:
		Clear();
		break;
	case ab::Workspace::notif::opened:
	case ab::Workspace::notif::added:
	case ab::Workspace::notif::shown:
		Load();
		break;
	case ab::Workspace::notif::hidden:
		break;
	default:
		break;
	}
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

		auto fut = sess->req(http::verb::get, "/product/get"s, std::move(body));
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
		wxMessageBox(exp.what(), "Products", wxICON_ERROR | wxOK);
		mBook->SetSelection(SERVER_ERROR);
	}
}

void ab::ProductView::SetupAuiTheme()
{
	auto artProvider = mManager.GetArtProvider();
	//pass the art provider to a place where it gets its settings from?
	ab::AuiTheme::Update(artProvider);
	ab::AuiTheme::Register(std::bind_front(&ab::ProductView::OnAuiThemeChange, this));
}

void ab::ProductView::OnAuiThemeChange()
{
	auto auiArt = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiArt);
	mManager.Update();
}
