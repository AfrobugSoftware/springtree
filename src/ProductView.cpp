#include "ProductView.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::ProductView, wxPanel)
	//tools
	EVT_TOOL(wxID_BACKWARD, ab::ProductView::OnBack)
	EVT_TOOL(wxID_FORWARD, ab::ProductView::OnForward)
	EVT_TOOL(ab::ProductView::ID_SELECT, ab::ProductView::OnSelect)
	EVT_TOOL(ab::ProductView::ID_ADD_PRODUCT, ab::ProductView::OnAddProduct)
	EVT_AUITOOLBAR_TOOL_DROPDOWN(ab::ProductView::ID_FORMULARY, ab::ProductView::OnFormularyToolbar)
	
	//updates
	EVT_UPDATE_UI(wxID_FORWARD, ab::ProductView::OnUpdateArrows)
	EVT_UPDATE_UI(wxID_BACKWARD, ab::ProductView::OnUpdateArrows)
	EVT_UPDATE_UI(ab::ProductView::ID_BOOK, ab::ProductView::OnUpdateBook)
	
	//menu
	EVT_MENU(ab::ProductView::ID_IMPORT_FORMULARY, ab::ProductView::OnImportFormulary)
	EVT_MENU(ab::ProductView::ID_EXPORT_FORMULARY, ab::ProductView::OnExportFormulary)
	
	//Search
	EVT_SEARCH(ab::ProductView::ID_SEARCH, ab::ProductView::OnSearch)
	EVT_SEARCH_CANCEL(ab::ProductView::ID_SEARCH, ab::ProductView::OnSearchCleared)
	EVT_TEXT(ab::ProductView::ID_SEARCH, ab::ProductView::OnSearch)

	//data view
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ab::ProductView::ID_DATA_VIEW, ab::ProductView::OnContextMenu)
	EVT_DATAVIEW_COLUMN_HEADER_CLICK(ab::ProductView::ID_DATA_VIEW, ab::ProductView::OnHeaderClick)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::ProductView::ID_DATA_VIEW, ab::ProductView::OnItemActivated)
	EVT_DATAVIEW_CACHE_HINT(ab::ProductView::ID_DATA_VIEW, ab::ProductView::OnCacheHint)
	
	EVT_TIMER(ab::ProductView::ID_SEARH_TIMER, ab::ProductView::OnSearchTimeOut)
END_EVENT_TABLE()


ab::ProductView::ProductView(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	:wxPanel(parent, id, position, size, style), mManager(this, ab::AuiTheme::AUIMGRSTYLE), mSearchTimer(this, ID_SEARH_TIMER)
{
	SetupAuiTheme();
	CreateBook();
	CreatePanels();
	CreateToolBar();
	CreateBottomTool();


	CreateView();
	CreateProductInfo();

	mManager.Update();
}

ab::ProductView::~ProductView()
{
	mManager.UnInit();
	mModel->Clear();
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

	ab::DataModel<ab::pproduct>::specialcol_t col;
	col.first = [&](wxVariant& v, int row, int col) {
		auto& arr = mModel->GetRow(row);
		v = std::format("{} {}", arr[7].GetString().ToStdString(), arr[8].GetString().ToStdString());
	};

	ab::DataModel<ab::pproduct>::specialcol_t SelectionCol;
	SelectionCol.first = [&](wxVariant& variant, size_t row, size_t col) {
		auto& rv = mModel->GetRow(row);
		auto found = mSelections.find(rv[0].GetString().ToStdString());
		variant = wxVariant((found != mSelections.end()));
	};

	SelectionCol.second = [&](const wxVariant& data, size_t row, size_t col) -> bool {
		auto& rv = mModel->GetRow(row);
		auto uuid = rv[0].GetString().ToStdString();
		if (data.GetBool()) {

			auto [iter, inserted] = mSelections.insert(uuid);
			return inserted;
		}
		else {
			mSelections.erase(uuid);
			return true;
		}
	};

	mModel->AddSpecialCol(std::move(SelectionCol), 1500);
	mModel->AddSpecialCol(std::move(col), col_strength);

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

	mBack = mTopTool->AddTool(wxID_BACKWARD,wxEmptyString, wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16,16))), "Back");
	mTopTool->AddSpacer(FromDIP(5));
	mForward = mTopTool->AddTool(wxID_FORWARD,wxEmptyString, wxArtProvider::GetBitmap("forward", wxART_OTHER, FromDIP(wxSize(16,16))), "forward");

	mSearchBar = new wxSearchCtrl(mTopTool, ID_SEARCH, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(450), FromDIP(-1)), wxWANTS_CHARS);
#ifndef __WXMAC__
	mSearchBar->ShowSearchButton(true);
#endif
	mSearchBar->ShowCancelButton(true);
	mSearchBar->SetDescriptiveText("Search for products");

	mTopTool->AddControl(mSearchBar);
	mTopTool->AddStretchSpacer();
	mTopTool->AddTool(ID_ADD_PRODUCT, "Add Product", wxArtProvider::GetBitmap("add", wxART_OTHER,FromDIP(wxSize(16, 16))));
	mTopTool->Realize();
	mManager.AddPane(mTopTool, wxAuiPaneInfo().Name("TopToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductView::CreateBottomTool()
{
	mBottomTool = new wxAuiToolBar(this, ID_BOTTOM_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mBottomTool->SetToolBitmapSize(wxSize(FromDIP(16), FromDIP(16)));

	auto select   = mBottomTool->AddTool(ID_SELECT, "Select", wxArtProvider::GetBitmap("select_check", wxART_OTHER, FromDIP(wxSize(16, 16))), "Select an Item", wxITEM_CHECK);
	mBottomTool->AddSpacer(FromDIP(10));
	mFormularyTool = mBottomTool->AddTool(ID_FORMULARY, "Formulary", wxArtProvider::GetBitmap("edit_note", wxART_OTHER, FromDIP(wxSize(16, 16))), "Formulary");
	mFormularyTool->SetHasDropDown(true);

	mBottomTool->Realize();
	mManager.AddPane(mBottomTool, wxAuiPaneInfo().Name("BottomToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(2).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductView::CreateProductInfo()
{
	mProductInfo = new ab::ProductInfo(mBook, wxID_ANY);
	mProductInfo->mOnBack = [&]() {
		auto& toptool = mManager.GetPane("TopToolBar");
		auto& bottoll = mManager.GetPane("BottomToolBar");
		if (!toptool.IsOk() || !bottoll.IsOk()) return;
		toptool.Show(true);
		bottoll.Show(true);
		mManager.Update();

		mProductInfo->UnLoad();
		mBook->SetSelection(VIEW);
	};
	mBook->AddPage(mProductInfo, "Product info", false);
}

void ab::ProductView::Load()
{
	//mBook->SetSelection(VIEW);
	size_t limit = mView->GetCountPerPage();
	if (limit == 0) limit = 22;
	mBook->SetSelection(WAIT);
	mActivity->Start();
	boost::asio::post(wxGetApp().mTaskManager.tp(), 
		std::bind(&ab::ProductView::GetProducts, this, 0, limit));
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
	if (mPageStack.empty() || mPageStack.size() == 1 || !mSearchBar->IsEmpty()) return;
	auto top = mPageStack.top();
	mPageStack.pop();

	boost::asio::post(wxGetApp().mTaskManager.tp(),
		std::bind(&ab::ProductView::GetProducts, this, top.first, top.second - top.first));
}

void ab::ProductView::OnForward(wxCommandEvent& evt)
{
	if (mPageStack.empty() || !mSearchBar->IsEmpty()) return;
	auto& curTop = mPageStack.top();
	auto newRange = std::make_pair(curTop.first - mView->GetCountPerPage(), mView->GetCountPerPage());

	boost::asio::post(wxGetApp().mTaskManager.tp(),
		std::bind(&ab::ProductView::GetProducts, this, newRange.first, newRange.second - newRange.first));
	mPageStack.push(newRange);

}

void ab::ProductView::OnImportFormulary(wxCommandEvent& evt)
{
	auto& app = wxGetApp();
	try{
		//read in a .form file
		wxFileDialog fileDialog(this, "Formulary Import", wxEmptyString, wxEmptyString, "form files (*.form)|*.form", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (fileDialog.ShowModal() == wxID_CANCEL)
			return;

		wxProgressDialog pdg("Loading formulary", "please wait...", 100, this, wxPD_SMOOTH | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
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


		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		grape::session::response_type resp{};
		std::string target = std::format("/product/formulary/checkname/{}", app.mPharmacyManager.pharmacy.name);
		grape::credentials cred;
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();
		constexpr const size_t cred_size = grape::serial::get_size(cred);
		grape::uid_t form_id;

		grape::body_type bt(cred_size, 0x00);
		grape::serial::write(boost::asio::buffer(bt), cred);
		auto fut = sess->req(http::verb::get, target, std::move(bt));
		pdg.Update(10,"Checking for formulary...");
		resp = fut.get();
	

		if (resp.result() == http::status::ok) {
			auto& body = resp.body();
			if (body.empty()) throw std::invalid_argument("expected a body");

			auto&& [id, buf] = grape::serial::read<grape::uid_t>(boost::asio::buffer(body));
			form_id = id;
		}
		else if (resp.result() == http::status::not_found) {
			//create a new formulary for 
			grape::formulary form;
			form.id           = boost::uuids::random_generator_mt19937{}();
			form.creator_id   = app.mPharmacyManager.pharmacy.id;
			form.created_by   = app.mPharmacyManager.account.username;
			form.access_level = grape::formulary_access_level::ACCESS_PRIVATE;
			form.usage_count  = 1ull;
			form.name         = app.mPharmacyManager.pharmacy.name;
			form.version      = "v1.0"s;
			const size_t size = grape::serial::get_size(form);
			grape::body_type bf(size + cred_size, 0x00);
			auto b = grape::serial::write(boost::asio::buffer(bf), cred);
			auto b2 = grape::serial::write(b, form);

			fut = sess->req(http::verb::post, "/product/formulary/create", std::move(bf));
			pdg.Update(15, "Creating a formulary...");
			resp = fut.get();


			if (resp.result() != http::status::ok) {
				//cannot create formulary for some reason
				throw std::logic_error(app.ParseServerError(resp));
			}
			boost::fusion::at_c<0>(form_id) = form.id;
		}
		else if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

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

		pdg.Update(30, "Loading imported formulary...");


		std::vector<grape::product> prods;
		std::vector<grape::pharma_product> pprods;
		const size_t size = products.size();
		prods.reserve(size);
		pprods.reserve(size);
		int i  = 0;
		int pg = 30;
		for (const auto& prod : products) {
			auto& p         = prods.emplace_back(grape::product{});
			p.id            = boost::uuids::random_generator_mt19937{}();
			p.serial_num    = 0;
			p.name          = static_cast<std::string>(prod["name"]);
			boost::trim(p.name);
			boost::to_lower(p.name);
			p.generic_name  = static_cast<std::string>(prod["generic_name"]);
			boost::trim(p.generic_name);
			boost::to_lower(p.generic_name);

			p.class_        = static_cast<std::string>(prod["class"]);
			p.formulation   = static_cast<std::string>(prod["formulation"]);
			p.strength      = static_cast<std::string>(prod["strength"]);
			p.strength_type = static_cast<std::string>(prod["strength_type"]);
			p.usage_info    = static_cast<std::string>(prod["usage_info"]);
			p.description   = static_cast<std::string>(prod["description"]);
			p.indications   = static_cast<std::string>(prod["health_conditions"]);
			p.sideeffects   = static_cast<std::string>(prod["side_effects"]);

			//add the product to the pharmacy
			auto& pp           = pprods.emplace_back(grape::pharma_product{});
			pp.branch_id       = app.mPharmacyManager.branch.id;
			pp.pharmacy_id     = app.mPharmacyManager.pharmacy.id;
			pp.product_id	   = p.id; //set the product id in grape juice
			pp.date_added      = std::chrono::system_clock::now();
			pp.unitprice       = pof::base::currency(static_cast<double>(prod["unit_price"]));
			pp.costprice       = pof::base::currency(static_cast<double>(prod["cost_price"]));
			pp.stock_count     = 0ull;
			pp.min_stock_count = 0ull;

			pg += static_cast<float>(((float)i / (float)size) * 30.f);
			pdg.Update(pg);
		}

		grape::collection_type<grape::product> pcollect;
		grape::collection_type<grape::pharma_product> ppcollect;

		boost::fusion::at_c<0>(pcollect)  = std::move(prods);
		boost::fusion::at_c<0>(ppcollect) = std::move(pprods);
		const size_t ss = cred_size + grape::serial::get_size(form_id)
			+ grape::serial::get_size(pcollect) + grape::serial::get_size(ppcollect);
		grape::body_type bss(ss, 0x00);
		auto bs1 = grape::serial::write(boost::asio::buffer(bss), cred);
		auto bs2 = grape::serial::write(bs1, form_id);
		auto bs3 = grape::serial::write(bs2, pcollect);
		auto bs4 = grape::serial::write(bs3, ppcollect);

		fut = sess->req(http::verb::post, "/product/formulary/import", std::move(bss), 5min);

		pdg.Update(80, "Loading products to grape juice....");
		resp = fut.get();

		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		Load(); //reload view
	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Formulary error:\n{}", exp.what()), "Formulary", wxICON_ERROR | wxOK);
		spdlog::error(exp.what());
		return;
	}
}

void ab::ProductView::OnExportFormulary(wxCommandEvent& evt)
{
}

void ab::ProductView::OnCreateFormulary(wxCommandEvent& evt)
{
}

void ab::ProductView::OnSearchCleared(wxCommandEvent& evt)
{
	Load(); //reload all data
}

void ab::ProductView::OnSearchTimeOut(wxTimerEvent& evt)
{
	auto text = mSearchBar->GetValue().ToStdString();
	if (text.empty()) return;

	if (!mStillSearching.load()) {
		mBook->SetSelection(WAIT);
		mActivity->Start();
		boost::asio::post(wxGetApp().mTaskManager.tp(),
			std::bind_front(&ab::ProductView::SearchProducts, this, std::move(text)));
	}
}

void ab::ProductView::OnSelect(wxCommandEvent& evt)
{
	if (mModel->empty()) return;
	if (evt.IsChecked())
	{
		mSelectCol = mView->PrependToggleColumn(wxT("Select"), 1500, wxDATAVIEW_CELL_ACTIVATABLE, FromDIP(50));
	}
	else 
	{
		mView->DeleteColumn(mSelectCol);
		mSelections.clear();
		mSelectCol = nullptr;
	}

}

void ab::ProductView::OnUpdateBook(wxUpdateUIEvent& evt)
{

}

void ab::ProductView::OnHeaderClick(wxDataViewEvent& evt)
{
}


void ab::ProductView::OnSearch(wxCommandEvent& evt)
{
	auto sText = evt.GetString().ToStdString();
	if (sText.empty()) {
		//reload
		return Load();	
	}

	if (std::ranges::all_of(sText, [](char s) {return std::isspace(s); }))
		return;

	if (!mSearchTimer.IsRunning()) {
		mSearchTimer.StartOnce(30); //wait 30ms to collect text
	}
}

void ab::ProductView::OnUpdateArrows(wxUpdateUIEvent& evt)
{
	wxWindowID id = evt.GetId();
	switch (id)
	{
	case wxID_BACKWARD:
	{
		auto sel = mBook->GetSelection();
		mBack->SetActive(sel == INFO);
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

void ab::ProductView::OnCacheHint(wxDataViewEvent& evt)
{
	size_t from = evt.GetCacheFrom();
	size_t to = evt.GetCacheTo();
	size_t count = to - from;

	spdlog::info("from {:d} To {:d}", from , to);
	if (mModel->size() >= to) 
		return;
	if (from < mModel->size()) 
		from = mModel->size();

	size_t viewSize = mView->GetCountPerPage();
	if (count < viewSize) 
		count = viewSize;

	boost::asio::post(wxGetApp().mTaskManager.tp(), 
		std::bind(&ab::ProductView::AppendProducts, this, from, count));

}

void ab::ProductView::OnItemActivated(wxDataViewEvent& evt)
{
	wxBusyCursor cusor;
	auto item = evt.GetItem();
	if (!item.IsOk()) return;

	//hide toobars
	auto& toptool = mManager.GetPane("TopToolBar");
	auto& bottoll = mManager.GetPane("BottomToolBar");
	if (!toptool.IsOk() || !bottoll.IsOk()) return;
	toptool.Show(false);
	bottoll.Show(false);
	mManager.Update();

	mProductInfo->mSelectedProduct = ab::make_struct<ab::pproduct>
		(mModel->GetRow(ab::DataModel<ab::pproduct>::FromDataViewItem(item)));
	mProductInfo->Load();

	mBook->SetSelection(INFO);
}


void ab::ProductView::OnFormularyToolbar(wxAuiToolBarEvent& evt)
{
	wxMenu* menu = new wxMenu;
	menu->Append(ID_CREATE_FORMULARY, "Create", nullptr);
	menu->Append(ID_IMPORT_FORMULARY, "Import", nullptr);
	menu->Append(ID_EXPORT_FORMULARY, "Export", nullptr);

	wxPoint pos = mFormularyTool->GetSizerItem()->GetPosition();
	wxSize sz = mFormularyTool->GetSizerItem()->GetSize();

	mBottomTool->PopupMenu(menu, wxPoint{ pos.x, pos.y + sz.y + 2 });
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
		if (mProductCount.load() == -1) {
			GetProductCount();
		}

		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
		grape::credentials cred;
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
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
		
		mView->Freeze();
		mModel->Reload(boost::fusion::at_c<0>(col), begin, begin + limit, mProductCount.load());

		mView->Thaw();

		mActivity->Stop();
		mBook->SetSelection(VIEW);
	}
	catch (const std::exception& exp) {
		spdlog::error(std::format("{} :{}", std::source_location::current(), exp.what()));
		wxMessageBox(exp.what(), "Products", wxICON_ERROR | wxOK);
		mBook->SetSelection(SERVER_ERROR);
	}
}

void ab::ProductView::AppendProducts(size_t from, size_t to)
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
		grape::credentials cred;
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();
		grape::page page{ (std::uint32_t)from, (std::uint32_t)to};

		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(page);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		grape::serial::write(buf, page);

		auto fut = sess->req(http::verb::get, "/product/get"s, std::move(body));
		auto resp = fut.get();
		if (resp.result() == http::status::not_found) {
			return;
		}
		else if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& b = resp.body();
		if (b.empty()) throw std::logic_error("no data receievd");
		auto&& [col, buf2] = grape::serial::read<grape::collection_type<ab::pproduct>>(boost::asio::buffer(b));

		//mView->Freeze();
		mModel->Append(boost::fusion::at_c<0>(col));
		//mView->Thaw();

	}
	catch (const std::exception& exp) {
		spdlog::error(std::format("{} :{}", std::source_location::current(), exp.what()));
		wxMessageBox(std::format("Cannot load more data {}", exp.what()), "Products", wxICON_ERROR | wxOK);
	}
}

void ab::ProductView::SearchProducts(std::string&& sstring)
{
	mStillSearching = true;
	try {
		auto& app = wxGetApp();

		grape::credentials cred;
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();
		

		boost::fusion::vector<std::uint32_t, std::string> searchT{ 0ul, std::forward<std::string>(sstring) };
		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(searchT);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		grape::serial::write(buf, searchT);
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());

		auto fut = sess->req(http::verb::get, "/product/search", std::move(body));
		auto resp = fut.get();
		if (resp.result() == http::status::not_found) {
			mBook->SetSelection(EMPTY);
			mStillSearching = false;
			return;
		}

		auto& b = resp.body();
		if (b.empty()) throw std::logic_error("no data receievd");
		auto&& [col, buf2] = grape::serial::read<grape::collection_type<ab::pproduct>>(boost::asio::buffer(b));
		auto& c = boost::fusion::at_c<0>(col);
		mView->Freeze();
		mModel->Reload(c, 0, c.size(), c.size());
		mView->Thaw();

		mActivity->Stop();
		mBook->SetSelection(VIEW);
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		//wxMessageBox(exp.what(), "Search products", wxICON_ERROR | wxOK);
		mNoConnectionText->SetLabel(exp.what());
		mBook->SetSelection(SERVER_ERROR);
	}
	mStillSearching = false;
}

void ab::ProductView::GetProductCount()
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred;
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();

		const size_t size = grape::serial::get_size(cred);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		auto fut = sess->req(http::verb::get, "/product/count", std::move(body));
		auto resp = fut.get();

		if (resp.result() == http::status::not_found) mProductCount = 0ll;
		else if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));

		auto& rbody = resp.body();
		if (rbody.empty())
			throw std::logic_error("No count received");

		auto&& [count, rbuf] = grape::serial::read<boost::fusion::vector<std::int64_t>>(boost::asio::buffer(rbody));
		mProductCount = boost::fusion::at_c<0>(count);
	}
	catch (const std::exception& exp) {
		spdlog::error(std::format("{}: {}", std::source_location::current(), exp.what()));
		std::rethrow_exception(std::current_exception());
	}
}

void ab::ProductView::GetFormularies()
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		constexpr const size_t size = grape::serial::get_size(cred);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		auto fut = sess->req(http::verb::get, "/product/formulary/get", std::move(body));
		auto resp = fut.get();

		if (resp.result() == http::status::not_found) mProductCount = 0ll;
		else if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));

		auto& rbody = resp.body();
		if (rbody.empty())
			throw std::logic_error("No count received");

		auto&& [collection, rdbuf] = 
			grape::serial::read<grape::collection_type<grape::formulary>>(boost::asio::buffer(rbody));
		mFormularies = std::move(boost::fusion::at_c<0>(collection));

	}
	catch (const std::exception& exp) 
	{
		spdlog::error(std::format("{}: {}", std::source_location::current(), exp.what()));
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
