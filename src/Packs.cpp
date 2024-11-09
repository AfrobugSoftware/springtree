#include "Packs.hpp"
#include "Application.hpp"
BEGIN_EVENT_TABLE(ab::Packs, wxDialog)
	EVT_LIST_ITEM_SELECTED(ab::Packs::ID_PACK_SELECT,    ab::Packs::OnPackSelected)
	EVT_LIST_ITEM_ACTIVATED(ab::Packs::ID_PACK_SELECT,   ab::Packs::OnPackActivate)
	EVT_LIST_ITEM_RIGHT_CLICK(ab::Packs::ID_PACK_SELECT, ab::Packs::OnRightClick)
	EVT_LIST_END_LABEL_EDIT(ab::Packs::ID_PACK_SELECT,   ab::Packs::OnEditPackName)

	EVT_TOOL(ab::Packs::ID_TOOL_ADD_PACK,            ab::Packs::OnAddPack)
	EVT_TOOL(ab::Packs::ID_TOOL_REMOVE_PRODUCT_PACK, ab::Packs::OnRemoveProductPack)
	EVT_TOOL(ab::Packs::ID_TOOL_GO_BACK,             ab::Packs::OnBack)
	EVT_TOOL(ab::Packs::ID_SALE_PACK,                ab::Packs::OnSalePack)

	EVT_MENU(ab::Packs::ID_OPEN_PACK,    ab::Packs::OnOpenPack)
	EVT_MENU(ab::Packs::ID_RENAME_PACK,  ab::Packs::OnRenamePack)
	EVT_MENU(ab::Packs::ID_REMOVE_PACK,  ab::Packs::OnRemovePack)

	EVT_SEARCH(ab::Packs::ID_PRODUCT_SEARCH_NAME,        ab::Packs::OnSearch)
	EVT_TEXT(ab::Packs::ID_PRODUCT_SEARCH_NAME,          ab::Packs::OnSearch)
	EVT_SEARCH_CANCEL(ab::Packs::ID_PRODUCT_SEARCH_NAME, ab::Packs::OnSearch)
END_EVENT_TABLE()

ab::Packs::Packs(wxWindow* parent, wxWindowID id, bool showSale, const std::string& title, const wxPoint& pos, const wxSize& size, long style)
: wxDialog(parent, id, title, pos, size, style), mShowSale(showSale), mManager(this, ab::AuiTheme::AUIMGRSTYLE){
	SetupAuiTheme();
	this->SetSize(FromDIP(wxSize(957, 542)));
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(*wxWHITE);

	mBook = new wxSimplebook(this, wxID_ANY);
	mManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").CenterPane().Show());

	CreateTopTools();
	CreatePackTools();

	CreatePanels();
	CreateSelectPanel();
	CreateView();

	mManager.Update();
	SetIcon(wxGetApp().mAppIcon);

	//can I do this here?
	wxBusyInfo wait("Loading packs\nPlease wait...");
	LoadPackDescSelect();
}

ab::Packs::~Packs()
{
}

void ab::Packs::SetupAuiTheme()
{
	auto art = mManager.GetArtProvider();
	ab::AuiTheme::Update(art);
	ab::AuiTheme::Register(std::bind_front(&ab::Packs::UpdateTheme, this));
}

void ab::Packs::UpdateTheme()
{
	auto art = mManager.GetArtProvider();
	ab::AuiTheme::Update(art);
}

void ab::Packs::CreatePanels()
{
	auto& app = wxGetApp();
	wxButton* addButton = nullptr;
	std::tie(mEmpty,std::ignore, addButton) = app.CreateEmptyPanel(mBook, "No packs in pharmacy");
	addButton->SetLabelText("Add pack");
	addButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		OnAddPack(evt);
	});

	std::tie(mEmptyPack, std::ignore, addButton) = app.CreateEmptyPanel(mBook, "No product in pack");
	addButton->SetLabelText("Add product");
	addButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		wxMessageBox("Search for product to add with the search bar", "Packs", wxICON_INFORMATION | wxOK);
		mSearch->SetFocus();
	});

	std::tie(mWaitPanel, mWaitIndicator) = app.CreateWaitPanel(mBook, "Please wait...");

	std::tie(mServerErrorPanel, mServerErrorText, addButton) = app.CreateEmptyPanel(mBook, "Server error", wxART_ERROR);
	addButton->SetLabel("Retry");
	addButton->SetBitmap(wxArtProvider::GetBitmap("retry", wxART_OTHER, FromDIP(wxSize(16,16))));
	

	mBook->AddPage(mEmpty, "Empty", false);
	mBook->AddPage(mEmptyPack, "EmptyPack", false);
	mBook->AddPage(mWaitPanel, "Wait panel", false);
	mBook->AddPage(mServerErrorPanel, "Error", false);
}


void ab::Packs::CreateTopTools()
{
	mTopTools = new wxAuiToolBar(this, ID_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW);
	mTopTools->SetToolBitmapSize(wxSize(16, 16));
	mTopTools->SetMinSize(wxSize(-1, 40));

	mTopTools->AddStretchSpacer();
	mTopTools->AddTool(ID_TOOL_ADD_PACK, "Add Pack", wxArtProvider::GetBitmap("add", wxART_OTHER, FromDIP(wxSize(16,16))), "Add a new pack");
	mTopTools->AddSpacer(FromDIP(5));
	mTopTools->AddTool(ID_RENAME_PACK, "Rename pack", wxArtProvider::GetBitmap("folder_manage", wxART_OTHER, FromDIP(wxSize(16,16))), "Rename the pack");
	mTopTools->AddSpacer(FromDIP(5));
	mTopTools->AddTool(ID_REMOVE_PACK, "Remove Pack", wxArtProvider::GetBitmap("delete", wxART_OTHER, FromDIP(wxSize(16,16))), "Remove the pack");

	
	if (mShowSale) {
		mTopTools->AddTool(ID_SALE_PACK, "Sell Pack", wxArtProvider::GetBitmap("shopping_cart", wxART_OTHER, FromDIP(wxSize(16,16))), "Sell pack");
	}

	mTopTools->Realize();
	mManager.AddPane(mTopTools, wxAuiPaneInfo().Name("TopToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::Packs::CreatePackTools()
{
	mPackTools = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW);
	mPackTools->SetToolBitmapSize(wxSize(16, 16));
	mPackTools->SetMinSize(wxSize(-1, 40));
	mPackTools->AddTool(ID_TOOL_GO_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mPackTools->AddSeparator();
	mPackTools->AddSpacer(FromDIP(5));
	mPackText = new wxStaticText(mPackTools, wxID_ANY, "TEST", wxDefaultPosition, wxDefaultSize, 0);
	mPackText->SetFont(wxFontInfo().AntiAliased().Bold());
	mPackText->SetBackgroundColour(*wxWHITE);
	mTextItem = mPackTools->AddControl(mPackText);

	mPackTools->AddSpacer(FromDIP(10));
	mSearch = new wxSearchCtrl(mPackTools, ID_PRODUCT_SEARCH_NAME, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(250, -1)));
	mSearch->SetHint("Search product by name to add");
	mSearch->ShowCancelButton(true);
	mSearch->Bind(wxEVT_CHAR, [&](wxKeyEvent& evt) {
		switch (evt.GetKeyCode()) {
		case WXK_DOWN:
			mSearchPopup->SetNext();
			break;
		case WXK_UP:
			mSearchPopup->SetNext(false);
			break;
		case WXK_RETURN:
			mSearchPopup->SetActivated();
			break;
		default:
			evt.Skip();
			break;
		}
		});
	mPackTools->AddControl(mSearch);

	mPackTools->AddStretchSpacer();
	mPackTools->AddTool(ID_TOOL_REMOVE_PRODUCT_PACK, wxT("Remove Product"), wxArtProvider::GetBitmap("delete", wxART_OTHER, FromDIP(wxSize(16, 16))), "Remove product from pack");

	mPackTools->Realize();
	mSearchPopup = new ab::SearchPopup(this);
	mSearchPopup->sProductSignal.connect(std::bind_front(&ab::Packs::AddProduct, this));
	mManager.AddPane(mPackTools, wxAuiPaneInfo().Name("PackToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false).Hide());
}

void ab::Packs::CreateView()
{
	mPackData  = new wxDataViewCtrl(mBook, ID_PACK_DATA, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);
	mPackModel = new packmodel_t();
	mPackData->AssociateModel(mPackModel);
	mPackModel->DecRef();

	mProductName     = mPackData->AppendTextColumn(wxT("Name"), 1,     wxDATAVIEW_CELL_INERT, FromDIP(500));
	mProductQuantity = mPackData->AppendTextColumn(wxT("Quantity"), 2, wxDATAVIEW_CELL_EDITABLE, FromDIP(100));
	mPackageSize     = mPackData->AppendTextColumn(wxT("Package Size"), 3);
	mPrice           = mPackData->AppendTextColumn(wxT("Price"), 4);
	mExtPrice        = mPackData->AppendTextColumn(wxT("Exact Price"), 5);


	mBook->AddPage(mPackData, "View", false);
}

void ab::Packs::CreateSelectPanel()
{
	mPackSelect = new wxListCtrl(mBook, ID_PACK_SELECT, wxDefaultPosition, wxDefaultSize, wxLC_ICON | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE | wxFULL_REPAINT_ON_RESIZE | wxLC_EDIT_LABELS | wxNO_BORDER);
	wxImageList* imagelist = new wxImageList(FromDIP(60), FromDIP(60));
	imagelist->Add(wxArtProvider::GetBitmap("cart", wxART_OTHER, FromDIP(wxSize(60,60))));
	mPackSelect->AssignImageList(imagelist, wxIMAGE_LIST_NORMAL);

	mBook->AddPage(mPackSelect, "Selection", false);
}

void ab::Packs::OnPackActivate(wxListEvent& evt)
{
	auto& app = wxGetApp();
	mSelectedItem = evt.GetItem();
	grape::pack& pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));
	SwitchTool();
	//set name 
	mPackTools->Freeze();
	mPackText->SetLabelText(pk.name);
	mTextItem->SetMinSize(mPackText->GetSize());
	mPackTools->Realize();
	mPackTools->Thaw();

	mPackModel->Clear();

	mWaitIndicator->Start();
	mBook->SetSelection(PACK_WAIT);
	boost::asio::post(app.mTaskManager.tp(), std::bind_front(&ab::Packs::LoadPackModel, this, pk.id));
}

void ab::Packs::OnEditPackName(wxListEvent& evt)
{
	if (evt.IsEditCancelled()) return;
	auto& app = wxGetApp();

	auto name = evt.GetLabel().ToStdString();
	if (name.empty()) {
		evt.Veto();
		return;
	}

	//check for duplicates
	for (int i = 0; i < mPackSelect->GetItemCount(); i++) {
		if (name == mPackSelect->GetItemText(i)) {
			wxMessageBox("Name already exists", "Pack", wxICON_WARNING | wxOK);
			evt.Veto();
			return;
		}
	}
	try {
		grape::pack& pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		boost::fusion::vector<boost::uuids::uuid, std::string> pkname{ pk.id, name };
		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(pkname);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, pkname);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::post, "/product/pack/rename", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Renaming pack\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			break;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		pk.name = std::move(name);
	}
	catch (const std::exception& exp) {
		evt.Veto();
		wxMessageBox(std::format("Cannot rename pack: {}", exp.what()), "Pack", wxICON_ERROR | wxOK);
	}
}

void ab::Packs::OnPackSelected(wxListEvent& evt)
{
	mSelectedItem = evt.GetItem();
}

void ab::Packs::OnAddPack(wxCommandEvent& evt)
{
	CHECK_PHARMACIST_PRIVILAGE();
	std::string packName = wxGetTextFromUser("Please enter pack name", "Pack").ToStdString();
	if (packName.empty()) return;
	boost::trim(packName);
	boost::to_lower(packName);

	auto& app = wxGetApp();
	grape::pack pk;
	pk.pharmacy_id = app.mPharmacyManager.pharmacy.id;
	pk.branch_id   = app.mPharmacyManager.branch.id;
	pk.name        = std::move(packName);
	pk.quantity    = 0ll;
	pk.cost        = pof::base::currency{};
	try {
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		const size_t size = grape::serial::get_size(cred) +
			grape::serial::get_size(pk);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, pk);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::post, "/product/pack/create", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Creating pack\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			break;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		
		//get back the pack id
		auto& rbody = resp.body();
		auto&& [pkid, rbuf] = grape::serial::read<grape::uid_t>(boost::asio::buffer(rbody));
		pk.id = boost::fusion::at_c<0>(pkid);

		//add pack to the select list
		if (mBook->GetSelection() != PACK_VIEW)
			mBook->SetSelection(PACK_VIEW);

		mPackSelect->Freeze();
		const size_t i = mPackSelect->GetItemCount();
		wxListItem item;
		item.SetId(i);
		item.SetData(new grape::pack{ pk });
		item.SetText(pk.name);
		item.SetImage(0);
		item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT | wxLIST_MASK_DATA);

		mPackSelect->InsertItem(std::move(item));
		mPackSelect->Thaw();
		mPackSelect->Refresh();
		
	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Cannot create pack: {}", exp.what()), "Pack", wxICON_ERROR | wxOK);
	}

}

void ab::Packs::OnRemovePack(wxCommandEvent& evt)
{
	CHECK_PHARMACIST_PRIVILAGE();
	if (mSelectedItem.GetId() == wxNOT_FOUND) {
		wxMessageBox("No item selected to remove", "Pack", wxICON_WARNING | wxOK);
		return;
	}
	if (wxMessageBox("Are you sure you want to remove pack?", "PACK", wxICON_WARNING | wxYES_NO) == wxNO) return;
	
	auto& app = wxGetApp();
	try {
		grape::pack pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));

		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		grape::uid_t pkid{ pk.id };

		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(pkid);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, pkid);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::post, "/product/pack/remove", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Removing product\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(PACK_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		mPackSelect->Freeze();
		mPackSelect->DeleteItem(mSelectedItem.GetId());
		mPackSelect->Thaw();

		if (mPackSelect->GetItemCount() == 0)
			mBook->SetSelection(PACK_EMPTY);
	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Cannot remove product from pack: {}", exp.what()), "Error in packs", wxICON_ERROR | wxOK);
		spdlog::error(exp.what());
	}
}

void ab::Packs::OnRemoveProductPack(wxCommandEvent& evt)
{
	CHECK_PHARMACIST_PRIVILAGE();
	auto& app = wxGetApp();
	auto item = mPackData->GetSelection();
	if (!item.IsOk()) {
		wxMessageBox("No item selected", "Pack", wxICON_WARNING | wxOK);
		return;
	}
	try {
		grape::pack pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));
		size_t idx = mPackModel->GetRow(item);
		auto& row  = mPackModel->GetRow(idx);
		grape::pack_product ppk{
		pk.id,
		boost::lexical_cast<boost::uuids::uuid>(row[0].GetString().ToStdString())};

		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(ppk);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, ppk);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::post, "/product/pack/products/remove", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Removing product\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(PACK_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		//remove item
		auto iter = std::next(mPackModel->begin(), idx);
		if (iter == mPackModel->end()) return;

		mPackModel->Remove(iter);

		if (mPackModel->empty())
			mBook->SetSelection(PACK_PRODUCT_EMPTY);
		//update ?


	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Cannot remove product from pack: {}", exp.what()), "Error in packs", wxICON_ERROR | wxOK);
		spdlog::error(exp.what());
	}
}

void ab::Packs::OnBack(wxCommandEvent& evt)
{
	SwitchTool();
	mBook->SetSelection(PACK_VIEW);
}

void ab::Packs::OnSalePack(wxCommandEvent& evt)
{
	if (mSelectedItem.GetId() == wxNOT_FOUND){
		wxMessageBox("No item selected for sale", "Pack", wxICON_WARNING | wxOK);
		return;
	}

	auto& app = wxGetApp();
	try {
		grape::pack pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		grape::uid_t pkid{ pk.id };
		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(pkid);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, pkid);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/pack/sale", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Adding pack to sale\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			wxMessageBox("Pack is empty,please sell a pack with products", "Pack", wxICON_WARNING | wxOK);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& rbody = resp.body();
		if (rbody.empty()) throw std::runtime_error("expected a body");
		auto&& [pps, rbuf] = grape::serial::read<grape::collection_type<grape::pharma_product>>(boost::asio::buffer(rbody));
		mSalePackProducts = std::move(boost::fusion::at_c<0>(pps));

		EndModal(wxID_OK);
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(std::format("Cannot sell pack: {}", exp.what()), "Error in packs", wxICON_ERROR | wxOK);
	}
}



void ab::Packs::OnOpenPack(wxCommandEvent& evt)
{
	auto& app = wxGetApp();
	grape::pack& pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));
	SwitchTool();
	//set name 
	mPackTools->Freeze();
	mPackText->SetLabelText(pk.name);
	mTextItem->SetMinSize(mPackText->GetSize());
	mPackTools->Realize();
	mPackTools->Thaw();

	mPackModel->Clear();

	mWaitIndicator->Start();
	mBook->SetSelection(PACK_WAIT);
	boost::asio::post(app.mTaskManager.tp(), std::bind_front(&ab::Packs::LoadPackModel, this, pk.id));
}

void ab::Packs::OnSearch(wxCommandEvent& evt)
{
	auto str = evt.GetString().ToStdString();
	if (str.empty()) {
		mSearchPopup->Dismiss();
		return;
	}

	wxPoint pos = mSearch->ClientToScreen(wxPoint(0, 0));
	wxSize sz   = mSearch->GetClientSize();

	mSearchPopup->SetPosition(wxPoint{ pos.x, pos.y + sz.y + 5 });
	mSearchPopup->SetSize(FromDIP(wxSize(sz.x + 500, 400)));

	mSearchPopup->Search(str);
	mSearchPopup->Popup();
}

void ab::Packs::AddProduct(const ab::pproduct& pp)
{
	CHECK_PHARMACIST_PRIVILAGE();
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };
		grape::pack pk = *(reinterpret_cast<grape::pack*>(mSelectedItem.GetData()));
		grape::pack_product ppk{
			pk.id,
			pp.id
		};

		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(ppk);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, ppk);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/pack/products/add", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(PACK_PRODUCT_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		packmodel_t::fusion_t f;
		boost::fusion::at_c<0>(f) = pp.id;
		boost::fusion::at_c<1>(f) = pp.name;
		boost::fusion::at_c<2>(f) = pp.package_size;
		boost::fusion::at_c<3>(f) = pp.stock_count;
		boost::fusion::at_c<4>(f) = pp.unit_price;
		mPackModel->Add(std::move(f));

		mBook->SetSelection(PACK_DATA);
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		mServerErrorText->SetLabelText(std::format("Cannot add product to pack: {}", exp.what()));
		mServerErrorPanel->Layout();
		mBook->SetSelection(PACK_SERVER_ERROR);
	}
}

void ab::Packs::SwitchTool()
{
	auto& top = mManager.GetPane("TopToolBar");
	auto& pack = mManager.GetPane("PackToolBar");
	if (top.IsShown()){
		top.Hide();
		pack.Show();
	}
	else {
		top.Show();
		pack.Hide();
	}
	mManager.Update();
}

void ab::Packs::LoadPackDescSelect()
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id};

		constexpr const size_t size = grape::serial::get_size(cred);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		
		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/pack/get", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(PACK_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) std::runtime_error("expected a body"s);

		auto&& [cp, buf] = grape::serial::read<grape::collection_type<grape::pack>>(boost::asio::buffer(rbody));
		auto& pks = boost::fusion::at_c<0>(cp);
		if (pks.empty()) return;

		mPackSelect->Freeze();
		for (auto& p : pks) {
			size_t i = mPackSelect->GetItemCount();
			wxListItem item;
			item.SetId(i);
			item.SetData(new grape::pack{p});
			item.SetText(p.name);
			item.SetImage(0);
			item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT | wxLIST_MASK_DATA);

			mPackSelect->InsertItem(std::move(item));
		}
		mPackSelect->Thaw();
		mBook->SetSelection(PACK_VIEW);

	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		mServerErrorText->SetLabelText(std::format("Cannot load pack: {}", exp.what()));
		mServerErrorPanel->Layout();
		mBook->SetSelection(PACK_SERVER_ERROR);
	}
}

void ab::Packs::LoadPackModel(boost::uuids::uuid uuid)
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id
		};
		
		grape::uid_t p{uuid};
		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(p);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, p);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/pack/products/get", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(PACK_PRODUCT_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) std::runtime_error("expected a body");

		auto&& [cp, buf] = grape::serial::read<grape::collection_type<boost::fusion::vector<
			boost::uuids::uuid,
			std::string,
			std::int64_t,
			std::int64_t,
			pof::base::currency
			>>>(boost::asio::buffer(rbody));
		auto& pks = boost::fusion::at_c<0>(cp);
		
		mPackData->Freeze();
		mPackModel->Reload(pks, 0, pks.size(), pks.size());
		mPackData->Thaw();

		mWaitIndicator->Stop();
		mBook->SetSelection(PACK_DATA);
	}
	catch (const std::exception& exp){
		//wxMessageBox(std::format("Cannot load pack: {}", exp.what()), "Error in packs", wxICON_ERROR | wxOK);
		spdlog::error(exp.what());
		mServerErrorText->SetLabelText(std::format("Cannot load pack products: {}", exp.what()));
		mServerErrorPanel->Layout();
		mBook->SetSelection(PACK_SERVER_ERROR);
	}
}

void ab::Packs::OnRightClick(wxListEvent& evt)
{
	mSelectedItem = evt.GetItem();
	wxMenu* menu = new wxMenu;
	auto op = menu->Append(ID_OPEN_PACK, "Open pack", nullptr);
	auto rn = menu->Append(ID_RENAME_PACK, "Rename", nullptr);
	auto rv = menu->Append(ID_REMOVE_PACK, "Remove", nullptr);

	op->SetBitmap(wxArtProvider::GetBitmap("file_open", wxART_OTHER, FromDIP(wxSize(16, 16))));
	rn->SetBitmap(wxArtProvider::GetBitmap("folder_manage", wxART_OTHER, FromDIP(wxSize(16, 16))));
	rv->SetBitmap(wxArtProvider::GetBitmap("delete", wxART_OTHER, FromDIP(wxSize(16, 16))));

	PopupMenu(menu);
}

void ab::Packs::OnRenamePack(wxCommandEvent& evt)
{
	CHECK_PHARMACIST_PRIVILAGE();
	if (mSelectedItem.GetId() == wxNOT_FOUND) {
		wxMessageBox("No item selected to rename", "Pack", wxICON_WARNING | wxOK);
		return;
	}
	mPackSelect->EditLabel(mSelectedItem.GetId());
}
