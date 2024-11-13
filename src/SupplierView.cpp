#include "SupplierView.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::SupplierView, wxPanel)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::SupplierView::ID_SUPPLIER_VIEW, ab::SupplierView::OnOpenSupplier)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::SupplierView::ID_INVOICE_VIEW,  ab::SupplierView::OnOpenInvoice)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ab::SupplierView::ID_INVOICE_PRODUCT_VIEW, ab::SupplierView::OnInvoiceProductContextMenu)
	EVT_SEARCH(ab::SupplierView::ID_PRODUCT_SEARCH, ab::SupplierView::OnProductSearch)
	EVT_TEXT(ab::SupplierView::ID_PRODUCT_SEARCH,   ab::SupplierView::OnProductSearch)
	EVT_TOOL(ab::SupplierView::ID_TOOL, ab::SupplierView::OnAddSupplier)
	EVT_TOOL(ab::SupplierView::ID_INVOICE_TOOL, ab::SupplierView::OnAddInvoice)
	EVT_TOOL(ab::SupplierView::ID_BACK, ab::SupplierView::OnBack)
	EVT_TOOL(ab::SupplierView::ID_SUPPLIER_BACK, ab::SupplierView::OnBack)
END_EVENT_TABLE()

ab::SupplierView::SupplierView(wxWindow* win, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	: wxPanel(win, id, position, size, style), mManager(this, ab::AuiTheme::AUIMGRSTYLE) {
	mBook = new wxSimplebook(this);
	SetSizeHints(wxDefaultSize, wxDefaultSize);
	SetBackgroundColour(*wxWHITE);
	SetDoubleBuffered(true);

	auto auiArtProvider = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiArtProvider);
	ab::AuiTheme::sSignal.connect(std::bind_front(&ab::SupplierView::OnAuiThemeChange, this));

	CreateToolBar();
	CreateViews();
	CreatePanels();

	page = SUPPLIER_VIEW;
	mSearchPopup = new ab::SearchPopup(this);
	mSearchPopup->sProductSignal.connect(std::bind_front(&ab::SupplierView::AddStock, this));
	mManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").CenterPane());
	mManager.Update();
}

void ab::SupplierView::Suppliers()
{
	mBook->SetSelection(WAIT_PANEL);
	mWaitIndicator->Start();
	auto& app = wxGetApp();
	boost::asio::post(app.mTaskManager.tp(),
		std::bind_front(&ab::SupplierView::LoadSuppliers, this, 0, 100));
	SwitchTool(SUPPLIER_VIEW);
}

void ab::SupplierView::OnBack(wxCommandEvent& evt)
{
	if (evt.GetId() == ID_SUPPLIER_BACK)
	{
		mOnBack();
	}
	else {
		switch (page)
		{
		case INVOICE_VIEW:
			mBook->SetSelection(SUPPLIER_VIEW);
			SwitchTool(SUPPLIER_VIEW);
			break;
		case INVOICE_PRODUCT_VIEW:
			mBook->SetSelection(INVOICE_VIEW);
			SwitchTool(INVOICE_VIEW);
			break;
		default:
			break;
		}
	}
}

void ab::SupplierView::OnAddSupplier(wxCommandEvent& evt)
{
	std::string str = wxGetTextFromUser("Please enter a supplier name", "Supplier").ToStdString();
	if (str.empty()) return;

	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		//check if name already exists
		boost::trim(str);
		boost::to_lower(str);
		grape::string_t nstr{ str };
		size_t size = grape::serial::get_size(cred, nstr);
		grape::body_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred, nstr);
		auto fut = sess->req(http::verb::get, "/product/supplier/check", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Checking supplier\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			wxMessageBox(std::format("{} already exists", str), "Supplier", wxICON_WARNING | wxOK);
			return;
		case http::status::not_found:
			break;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		grape::supplier supp;
		supp.pharm_id  = cred.pharm_id;
		supp.branch_id = cred.branch_id;
		supp.name      = str;
		
		size      = grape::serial::get_size(cred, supp);
		body      = grape::body_type( size, 0x00 );
		auto buf2 = grape::serial::write(boost::asio::buffer(body), cred, supp);

		fut = sess->req(http::verb::post, "/product/supplier/create", std::move(body));
		{
			wxBusyInfo wait("Adding supplier\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& rbody        = resp.body();
		auto&& [ret, rbuf] = grape::serial::read<grape::supplier>(boost::asio::buffer(rbody));

		supp.date_created  = ret.date_created;
		supp.date_modified = ret.date_modified;
		supp.id            = ret.id;

		mSupplierModel->Add(std::move(supp));
		mBook->SetSelection(SUPPLIER_VIEW);
	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Error adding supplier: {}", exp.what()), "Supplier", wxICON_ERROR | wxOK);
	}
}

void ab::SupplierView::OnAddInvoice(wxCommandEvent& evt)
{
	std::string str = wxGetTextFromUser("Please enter an invoice id", "Supplier").ToStdString();
	if (str.empty()) return;
	
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		boost::trim(str);
		boost::to_lower(str);
		grape::string_t nstr{ str };
		size_t size = grape::serial::get_size(cred, nstr);
		grape::body_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred, nstr);

		auto fut = sess->req(http::verb::get, "/product/invoice/check", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Checking invoice\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			wxMessageBox(std::format("{} already exists", str), "Supplier", wxICON_WARNING | wxOK);
			return;
		case http::status::not_found:
			break;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		grape::invoice inv;
		inv.pharm_id    = cred.pharm_id;
		inv.branch_id   = cred.branch_id;
		inv.supplier_id = mCurSupp.id;
		inv.name        = str;
		inv.id			= boost::uuids::nil_uuid();
		inv.product_id  = boost::uuids::nil_uuid(); //an invoice entry with a nil product would signify the name of the invoice, first entry
		
		size = grape::serial::get_size(cred, inv);
		body = grape::body_type(size, 0x00);
		fut  = sess->req(http::verb::post, "/product/invoice/add", std::move(body));
		{
			wxBusyInfo wait("Adding invoice\nPlease wait...");
			resp = fut.get();
		}
		switch (resp.result())
		{
		case http::status::ok:
			return;
		case http::status::not_found:
			break;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& rbody = resp.body();
		auto&& [id, rbuf] = grape::serial::read<grape::uid_t>(boost::asio::buffer(rbody));
		inv.id = boost::fusion::at_c<0>(id);

		mInvoiceModel->Add(invoice_t::fusion_t{ inv.id, inv.name, std::chrono::system_clock::now() });
		mBook->SetSelection(INVOICE_VIEW);

	}
	catch (const std::exception& exp) {
		wxMessageBox(std::format("Error adding invoice: {}", exp.what()), "Supplier", wxICON_ERROR | wxOK);
	}
}

void ab::SupplierView::OnInvoiceContextMenu(wxDataViewEvent& evt)
{
	wxMenu* menu = new wxMenu;

	mInvoiceView->PopupMenu(menu);
}

void ab::SupplierView::OnOpenSupplier(wxDataViewEvent& evt)
{
	CHECK_PHARMACIST_PRIVILAGE();
	const auto item   = evt.GetItem();
	const size_t idx  = mSupplierModel->GetRow(item);
	mCurSupp          = ab::make_struct<grape::supplier>(mSupplierModel->GetRow(idx));

	mBook->SetSelection(WAIT_PANEL);
	mWaitIndicator->Start();
	auto& app = wxGetApp();
	boost::asio::post(app.mTaskManager.tp(),
		std::bind_front(&ab::SupplierView::LoadInvoice, this, mCurSupp.id , 0, 100));
	SwitchTool(INVOICE_VIEW);
}

void ab::SupplierView::OnOpenInvoice(wxDataViewEvent& evt)
{
	auto item = evt.GetItem();
	const size_t idx = mInvoiceModel->GetRow(item);
	mCurInvoice = ab::make_struct<invoice_t::fusion_t>(mInvoiceModel->GetRow(idx));

	mBook->SetSelection(WAIT_PANEL);
	mWaitIndicator->Start();
	auto& app = wxGetApp();
	boost::asio::post(app.mTaskManager.tp(),
		std::bind_front(&ab::SupplierView::LoadInvoiceProducts, this, boost::fusion::at_c<0>(mCurInvoice)));
	SwitchTool(INVOICE_PRODUCT_VIEW);
}

void ab::SupplierView::OnInvoiceProductContextMenu(wxDataViewEvent& evt)
{

}

void ab::SupplierView::OnProductSearch(wxCommandEvent& evt)
{
	auto str = evt.GetString();
	if (str.empty()) {
		mSearchPopup->Dismiss();
		return;
	}

	wxPoint pos = mInvoiceProductSearch->ClientToScreen(wxPoint(0, 0));
	wxSize sz   = mInvoiceProductSearch->GetClientSize();

	mSearchPopup->SetPosition(wxPoint{ pos.x, pos.y + sz.y + 5 });
	mSearchPopup->SetSize(FromDIP(wxSize(sz.x + 500, 400)));

	mSearchPopup->Search(str.ToStdString());
	mSearchPopup->Popup();
}


void ab::SupplierView::AddStock(const ab::pproduct& prod)
{
	auto& app = wxGetApp();
	wxDialog dialog(this, wxID_ANY, "Add stock");
	auto d = std::addressof(dialog);
	d->SetSize(FromDIP(wxSize(591 , 398)));
	d->SetBackgroundColour(*wxWHITE);
	d->ClearBackground();

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	wxStdDialogButtonSizer* mButtonSizer = new wxStdDialogButtonSizer();
	wxButton* m_sdbSizer2OK = new wxButton(d, wxID_OK);
	mButtonSizer->AddButton(m_sdbSizer2OK);
	wxButton* m_sdbSizer2Cancel = new wxButton(d, wxID_CANCEL);
	mButtonSizer->AddButton(m_sdbSizer2Cancel);
	mButtonSizer->Realize();

	wxFlexGridSizer* flexSizer = new wxFlexGridSizer(8, 3, FromDIP(2), FromDIP(5));
	flexSizer->AddGrowableCol(1);
	flexSizer->SetFlexibleDirection(wxBOTH);
	flexSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxStaticText* Title = new wxStaticText(d, wxID_ANY, std::format("Add Stock - {}", prod.name));
	Title->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS).Bold()));
	wxStaticText* Description = new wxStaticText(d, wxID_ANY, "Adds stock to the product for sale");


	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Batch No"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxTextCtrl* mBatchNumber = new wxTextCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)));
	mBatchNumber->SetValidator(wxTextValidator{ wxFILTER_DIGITS | wxFILTER_EMPTY });
	flexSizer->Add(mBatchNumber, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();

	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Quantity"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxSpinCtrl* mQuantityInControl = new wxSpinCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)), wxSP_ARROW_KEYS | wxALIGN_LEFT, 0, std::numeric_limits<int>::max());
	flexSizer->Add(mQuantityInControl, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();

	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Expiry Date"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxDatePickerCtrl* mExpiryDate = new wxDatePickerCtrl(d, wxID_ANY, wxDateTime::Now(), wxDefaultPosition, FromDIP(wxSize(200, -1)), wxDP_DROPDOWN);
	wxDateTime dt;
	ab::validator<wxDatePickerCtrl> mDateValidator(&dt);
	mDateValidator.OnValidate = [&](wxDatePickerCtrl* picker) -> bool {
		auto expDate = date::floor<date::days>(std::chrono::system_clock::from_time_t(mExpiryDate->GetValue().GetTicks()));
		auto nowDate = date::floor<date::days>(pof::base::data::clock_t::now()) - date::days(1);
		if (expDate == nowDate) {
			wxMessageBox("Expiry date cannot be today's date, check and try again", "Add Stock", wxICON_INFORMATION | wxOK);
			return false;
		}
		return true;
		};
	mExpiryDate->SetRange(wxDateTime::Now(), wxDateTime{});
	//	mExpiryDate->SetValidator(mDateValidator);
	flexSizer->Add(mExpiryDate, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();

	float fv = 0.0f;
	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Cost Price"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxTextCtrl* mCostControl = new wxTextCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)));
	wxFloatingPointValidator<float> val(2, &fv, wxNUM_VAL_ZERO_AS_BLANK);
	val.SetRange(0, 999999999999);
	mCostControl->SetValidator(val);
	flexSizer->Add(mCostControl, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();

	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Sale Price"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxTextCtrl* mUnitControl = new wxTextCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)));
	val.SetRange(0, 999999999999);
	mUnitControl->SetValidator(val);
	flexSizer->Add(mUnitControl, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();


	boxSizer->Add(Title, wxSizerFlags().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	boxSizer->Add(Description, wxSizerFlags().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	boxSizer->Add(flexSizer, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	boxSizer->Add(mButtonSizer, wxSizerFlags().Expand().Border(wxALL, FromDIP(5)));
	topSizer->Add(boxSizer, wxSizerFlags().Expand().Border(wxALL, FromDIP(5)));
	d->SetSizer(topSizer);
	//topSizer->SetSizeHints(this);
	d->Center();
	d->SetIcon(app.mAppIcon);
	if (d->ShowModal() != wxID_OK)
		return;
	try {
		grape::inventory inven;
		inven.pharmacy_id = app.mPharmacyManager.pharmacy.id;
		inven.branch_id = app.mPharmacyManager.branch.id;
		inven.product_id = prod.id;
		inven.input_date = std::chrono::system_clock::now();
		inven.cost = pof::base::currency(boost::lexical_cast<float>(mCostControl->GetValue()));
		inven.expire_date = std::chrono::system_clock::from_time_t(mExpiryDate->GetValue().GetTicks());
		inven.lot_number = mBatchNumber->GetValue().ToStdString();
		inven.stock_count = static_cast<std::uint64_t>(mQuantityInControl->GetValue());

		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		grape::credentials cred{
		app.mPharmacyManager.account.account_id,
		app.mPharmacyManager.account.session_id.value(),
		app.mPharmacyManager.pharmacy.id,
		app.mPharmacyManager.branch.id };

		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(inven);
		grape::body_type body(size, 0x00);
		auto wbuf = grape::serial::write(boost::asio::buffer(body), cred);
		auto wbuf2 = grape::serial::write(wbuf, inven);

		auto fut = sess->req(http::verb::post, "/product/inventory/add", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Adding inventory to product\nPlease wait...");
			resp = fut.get();
		}
		if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));

		auto& rbody = resp.body();
		auto&& [invenID, rbuf] = grape::serial::read<grape::uid_t>(boost::asio::buffer(rbody));

		//update the product stock
		grape::pharma_product_opt pharma_opt;
		pharma_opt.branch_id   = cred.branch_id;
		pharma_opt.pharmacy_id = cred.pharm_id;
		pharma_opt.product_id  = prod.id;
		pharma_opt.stock_count = inven.stock_count;
		pharma_opt.unitprice   = pof::base::currency(boost::lexical_cast<double>(mUnitControl->GetValue().ToStdString()));
		pharma_opt.costprice   = pof::base::currency(boost::lexical_cast<double>(mCostControl->GetValue().ToStdString()));

		const size_t size2 = grape::serial::get_size(cred) + grape::serial::get_size(pharma_opt);
		grape::body_type body2(size2, 0x00);
		auto wbuf5 = grape::serial::write(boost::asio::buffer(body2), cred);
		auto wbuf6 = grape::serial::write(wbuf5, pharma_opt);

		fut = sess->req(http::verb::post, "/product/updatepharma", std::move(body2));
		{
			wxBusyInfo wait("Updating product stock\nPlease wait...");
			resp = fut.get();
		}

		if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));

		grape::invoice inv;
		inv.pharm_id       = cred.pharm_id;
		inv.branch_id	   = cred.branch_id;
		inv.supplier_id    = mCurSupp.id;
		inv.id             = boost::fusion::at_c<0>(mCurInvoice);
		inv.product_id     = prod.id;
		inv.name		   = boost::fusion::at_c<1>(mCurInvoice);
		inv.prod_name      = prod.name;
		inv.cost           = inven.cost;
		inv.inventory_id   = boost::fusion::at_c<0>(invenID);

		const size_t size3 = grape::serial::get_size(cred) + grape::serial::get_size(inv);
		grape::body_type body3(size2, 0x00);
		auto wbuf7 = grape::serial::write(boost::asio::buffer(body3), cred);
		auto wbuf8 = grape::serial::write(wbuf7, inv);
		fut = sess->req(http::verb::post, "/product/invoice/add", std::move(body3));
		{
			wxBusyInfo wait("Adding product to invoice\nPlease wait...");
			resp = std::move(fut.get());
		}
		if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));

		mInvoiceProductModel->Add(std::move(inv));
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(exp.what(), "Add stock", wxICON_ERROR | wxOK);
	}
}

void ab::SupplierView::OnAuiThemeChange()
{
	auto auiArtProvider = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiArtProvider);
}

void ab::SupplierView::CreateToolBar()
{
	mTools = new wxAuiToolBar(this, ID_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mTools->SetToolBitmapSize(FromDIP(wxSize(16, 16)));
	mTools->AddSpacer(FromDIP(5));
	mTools->AddTool(ID_SUPPLIER_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16, 16))), "Back");

	mTools->AddSpacer(5);
	mSupplierSearch = new wxSearchCtrl(mTools, ID_SEARCH, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(400, -1)), wxWANTS_CHARS);
	mTools->AddControl(mSupplierSearch);

	mTools->AddStretchSpacer();
	mTools->AddSeparator();
	mTools->AddTool(ID_ADD_SUPPLIER, "Create new supplier", wxArtProvider::GetBitmap("add", wxART_OTHER, FromDIP(wxSize(16, 16))), "Create supplier");
	mTools->AddSpacer(FromDIP(5));
	mTools->Realize();
	mManager.AddPane(mTools, wxAuiPaneInfo().Name("Tools").Top().MinSize(FromDIP(wxSize(-1, 30))).PaneBorder(false).ToolbarPane().Top().DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));

	mInvoiceTools = new wxAuiToolBar(this, ID_INVOICE_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mInvoiceTools->SetToolBitmapSize(FromDIP(wxSize(16, 16)));
	mInvoiceTools->AddTool(ID_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16, 16))), "Back");

	mSupplierName = new wxStaticText(mInvoiceTools, wxID_ANY, wxEmptyString , wxDefaultPosition, wxDefaultSize, 0);
	mSupplierName->SetFont(wxFontInfo().AntiAliased().Bold());
	mSupplierName->SetBackgroundColour(*wxWHITE);

	mInvoiceTools->AddSeparator();
	mInvoiceTools->AddSpacer(FromDIP(5));
	mName1 = mInvoiceTools->AddControl(mSupplierName);

	mInvoiceTools->AddStretchSpacer();
	mInvoiceTools->AddTool(ID_CREATE_INVOICE, "Add new invoice", wxArtProvider::GetBitmap("add", wxART_OTHER, FromDIP(wxSize(16, 16))), "Add a new invoice");
	mInvoiceTools->Realize();
	mManager.AddPane(mInvoiceTools, wxAuiPaneInfo().Name("InvoiceTools").Top().MinSize(FromDIP(wxSize(-1, 30))).PaneBorder(false).ToolbarPane().Top().DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false).Hide());

	mInvoiceProductTools = new wxAuiToolBar(this, ID_INVOICE_PRODUCT_TOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mInvoiceProductTools->SetToolBitmapSize(FromDIP(wxSize(16, 16)));

	mInvoiceProductSearch = new wxSearchCtrl(mInvoiceProductTools, ID_PRODUCT_SEARCH, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(400, -1)), wxWANTS_CHARS);
	mInvoiceProductTools->AddTool(ID_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16, 16))), "Back");
	mInvoiceProductTools->AddSpacer(FromDIP(5));

	mSupplierInvoiceName = new wxStaticText(mInvoiceProductTools, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mSupplierInvoiceName->SetFont(wxFontInfo().AntiAliased().Bold());
	mSupplierInvoiceName->SetBackgroundColour(*wxWHITE);

	mName2 = mInvoiceProductTools->AddControl(mSupplierInvoiceName);
	mInvoiceProductTools->AddSpacer(FromDIP(5));

	mInvoiceProductTools->AddSeparator();
	mInvoiceProductTools->AddSpacer(FromDIP(5));
	mInvoiceProductTools->AddControl(mInvoiceProductSearch);

	mInvoiceProductTools->Realize();
	mManager.AddPane(mInvoiceProductTools, wxAuiPaneInfo().Name("InvoiceProductTools").Top().MinSize(FromDIP(wxSize(-1, 30))).PaneBorder(false).ToolbarPane().Top().DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false).Hide());
}

void ab::SupplierView::CreatePanels()
{
	auto& app = wxGetApp();
	wxButton* addButton = nullptr;
	std::tie(mEmpty, std::ignore, addButton) = app.CreateEmptyPanel(mBook, "No supplier in pharmacy");
	addButton->SetLabelText("Add supplier");
	addButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		OnAddSupplier(evt);
	});

	std::tie(mEmptyInvoice, std::ignore, addButton) = app.CreateEmptyPanel(mBook, "No invoice in supplier");
	addButton->SetLabelText("Add Invoice");
	addButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		OnAddInvoice(evt);
	});

	std::tie(mEmptyInvoiceProduct, std::ignore, addButton) = app.CreateEmptyPanel(mBook, "No product in invoice");
	addButton->SetLabelText("Add Product");
	addButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {

	});

	std::tie(mWaitPanel, mWaitIndicator) = app.CreateWaitPanel(mBook, "Please wait...");

	std::tie(mServerErrorPanel, mServerErrorText, addButton) = app.CreateEmptyPanel(mBook, "Server error", wxART_ERROR);
	addButton->SetLabel("Retry");
	addButton->SetBitmap(wxArtProvider::GetBitmap("retry", wxART_OTHER, FromDIP(wxSize(16, 16))));


	mBook->AddPage(mEmpty, "Empty", false);
	mBook->AddPage(mEmptyInvoice, "EmptyInvoice", false);
	mBook->AddPage(mEmptyInvoiceProduct, "EmptyInvoiceProduct", false);
	mBook->AddPage(mWaitPanel, "Wait panel", false);
	mBook->AddPage(mServerErrorPanel, "Error", false);
}

void ab::SupplierView::CreateViews()
{
	//supplier view
	mSupplierView = new wxDataViewCtrl(mBook, ID_SUPPLIER_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mSupplierModel = new supplier_t();
	mSupplierView->AssociateModel(mSupplierModel);
	mSupplierModel->DecRef();


	mSupplierView->AppendTextColumn("Supplier Name", 0, wxDATAVIEW_CELL_INERT, FromDIP(450), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mSupplierView->AppendTextColumn("Date created",  1, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mSupplierView->AppendTextColumn("Date modified", 2, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);

	mBook->AddPage(mSupplierView, "View", false);

	mInvoiceView = new wxDataViewCtrl(mBook, ID_INVOICE_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mInvoiceView->AppendTextColumn("Invoices", 0, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInvoiceView->AppendTextColumn("Date",     1, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInvoiceModel = new invoice_t();
	mInvoiceView->AssociateModel(mInvoiceModel);
	mInvoiceModel->DecRef();

	mBook->AddPage(mInvoiceView, "Invoice", false);

	wxPanel* panel = new wxPanel(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxSizer* sz = new wxBoxSizer(wxVERTICAL);

	mInvoiceProductView = new wxDataViewCtrl(panel, ID_INVOICE_PRODUCT_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mInvoiceProductModel = new product_t();
	mInvoiceProductView->AssociateModel(mInvoiceProductModel);
	
	mInvoiceProductModel->DecRef();


	mInvoiceProductView->AppendTextColumn("Product", 0, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInvoiceProductView->AppendTextColumn("Stock entry", 1, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInvoiceProductView->AppendTextColumn("Cost", 2, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInvoiceProductView->AppendTextColumn("Entry date", 3, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInvoiceProductView->AppendTextColumn("Expiry date", 4, wxDATAVIEW_CELL_INERT, FromDIP(250), wxALIGN_LEFT, wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);

	mCSPanel = new wxPanel(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);

	bSizer4->AddStretchSpacer();

	mTotalStock = new wxStaticText(mCSPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mTotalStock->SetFont(wxFont(wxFontInfo(12).Bold().AntiAliased()));
	mTotalStock->Wrap(-1);
	bSizer4->Add(mTotalStock, 0, wxALL, FromDIP(5));

	bSizer4->AddSpacer(5);

	bSizer4->Add(new wxStaticLine(mCSPanel, -1, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), wxSizerFlags().Expand());

	bSizer4->AddSpacer(FromDIP(5));

	mTotalAmount = new wxStaticText(mCSPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mTotalAmount->SetFont(wxFont(wxFontInfo(12).Bold().AntiAliased()));
	mTotalAmount->Wrap(-1);
	bSizer4->Add(mTotalAmount, 0, wxALL, FromDIP(5));

	mCSPanel->SetSizer(bSizer4);
	mCSPanel->Layout();
	bSizer4->Fit(mCSPanel);

	sz->Add(mInvoiceProductView, 1, wxEXPAND | wxALL, FromDIP(0));
	sz->Add(mCSPanel, 0, wxEXPAND | wxALL, FromDIP(0));
	sz->AddSpacer(5);

	panel->SetSizer(sz);
	sz->SetSizeHints(panel);
	panel->Layout();
	mBook->AddPage(panel, "Products", false);
}

void ab::SupplierView::SwitchTool(int page)
{
	auto& sp  = mManager.GetPane("Tools");
	auto& ip  = mManager.GetPane("InvoiceTools");
	auto& ipp = mManager.GetPane("InvoiceProductTools");

	sp.Hide();
	ip.Hide();
	ipp.Hide();

	this->page = page;
	switch (page)
	{
	case INVOICE_VIEW:
	{
		mInvoiceTools->Freeze();
		mSupplierName->SetLabel(mCurSupp.name);
		mName1->SetMinSize(mSupplierName->GetSize());
		mInvoiceTools->Realize();
		mInvoiceTools->Thaw();
		ip.Show();
	}
		break;
	case SUPPLIER_VIEW:
		sp.Show();
		break;
	case INVOICE_PRODUCT_VIEW:
	{
		mInvoiceProductTools->Freeze();
		mSupplierInvoiceName->SetLabel(std::format("{} - {}", mCurSupp.name,
			boost::fusion::at_c<1>(mCurInvoice)));
		mName2->SetMinSize(mSupplierInvoiceName->GetSize());
		mInvoiceProductTools->Realize();
		mInvoiceProductTools->Thaw();
		ipp.Show();
	}
		break;
	default:
		break;
	}
	mManager.Update();
}

void ab::SupplierView::LoadSuppliers(int start, int end)
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};
		grape::page pg{ (std::uint32_t)start, (std::uint32_t)end };
		constexpr const size_t size = grape::serial::get_size(cred) +
			grape::serial::get_size(pg);
		grape::body_type body(size, 0x00);
		auto buf  = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf, pg);
		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/supplier/get", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(SUPPLIER_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) throw std::logic_error("expected a body");

		auto&& [supps, rbuf] = grape::serial::read<
			grape::collection_type<grape::supplier>>(boost::asio::buffer(rbody));
		mWaitIndicator->Stop();
		mSupplierModel->Reload(boost::fusion::at_c<0>(supps), start, end, start + end);

	}
	catch (const std::exception& exp) {
		mServerErrorText->SetLabel(std::format("failed to loaded invoice\n{}", exp.what()));
		mBook->SetSelection(SERVER_ERROR);
		mServerErrorPanel->Layout();
	}
}

void ab::SupplierView::LoadInvoice(boost::uuids::uuid suppid, int start, int end)
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};

		grape::uid_t sid{ suppid };
		grape::page pg{ (std::uint32_t)start, (std::uint32_t)end };
		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(sid)
			 + grape::serial::get_size(pg);
		grape::body_type body(size, 0x00);

		auto buf  = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf, sid);
		auto buf3 = grape::serial::write(buf2, pg);
		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/invoice/get", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(SUPPLIER_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) throw std::logic_error("expected a body");
		auto&& [invs, rbuf] = grape::serial::read<
			grape::collection_type<invoice_t::fusion_t>>(boost::asio::buffer(rbody));
		auto& i = boost::fusion::at_c<0>(invs);
		
		mWaitIndicator->Stop();
		mInvoiceModel->Reload(i, start, end, i.size());
		mBook->SetSelection(INVOICE_VIEW);
	}
	catch (const std::exception& exp)
	{
		mServerErrorText->SetLabel(std::format("failed to loaded invoice\n{}", exp.what()));
		mBook->SetSelection(SERVER_ERROR);
		mServerErrorPanel->Layout();
	}
}

void ab::SupplierView::LoadInvoiceProducts(boost::uuids::uuid invoiceID)
{
	auto& app = wxGetApp();
	try {
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};
		grape::collection_type<grape::uid_t> ids;
		auto& is = boost::fusion::at_c<0>(ids);
		is.emplace_back(grape::uid_t{ mCurSupp.id });
		is.emplace_back(grape::uid_t{ invoiceID });

		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(ids);
		grape::body_type body(size, 0x00);

		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf, ids);
		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/product/invoice/getproducts", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(INVOICE_EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) throw std::logic_error("expected a body");
		auto&& [invs, rbuf] = grape::serial::read<
			grape::collection_type<grape::invoice>>(boost::asio::buffer(rbody));
		auto& i = boost::fusion::at_c<0>(invs);

		mWaitIndicator->Stop();
		mInvoiceProductModel->Reload(i, 0, i.size(), i.size());
		mBook->SetSelection(INVOICE_VIEW);
	}
	catch (const std::exception& exp)
	{
		mServerErrorText->SetLabel(std::format("failed to loaded invoice products\n{}", exp.what()));
		mBook->SetSelection(SERVER_ERROR);
		mServerErrorPanel->Layout();
	}
}
