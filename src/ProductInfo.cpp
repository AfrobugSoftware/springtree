#include "ProductInfo.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::ProductInfo, wxPanel)
	EVT_TOOL(ab::ProductInfo::ID_TOOL_ADD_INVENTORY, ab::ProductInfo::OnAddStock)
	EVT_TOOL(ab::ProductInfo::ID_ADD_BARCODE, ab::ProductInfo::OnAddBarcode)
	EVT_TOOL(ab::ProductInfo::ID_SHOW_PRODUCT_SALE_HISTORY, ab::ProductInfo::OnHistory)
	EVT_TOOL(ab::ProductInfo::ID_TOOL_SHOW_PRODUCT_INFO, ab::ProductInfo::OnProductProperty)
	EVT_TOOL(ab::ProductInfo::ID_TOOL_GO_BACK, ab::ProductInfo::OnBack)
	EVT_PG_CHANGING(ab::ProductInfo::ID_PROPERTY_GRID, ab::ProductInfo::OnPropertyChanging)
	EVT_PG_CHANGED(ab::ProductInfo::ID_PROPERTY_GRID, ab::ProductInfo::OnPropertyChanged)
	EVT_DATE_CHANGED(ab::ProductInfo::ID_INVEN_START_DATE_PICKER, ab::ProductInfo::OnDateChange)
	EVT_DATE_CHANGED(ab::ProductInfo::ID_INVEN_STOP_DATE_PICKER, ab::ProductInfo::OnDateChange)
	EVT_DATAVIEW_CACHE_HINT(ab::ProductInfo::ID_DATA_VIEW, ab::ProductInfo::OnCacheHint)
END_EVENT_TABLE()


ab::ProductInfo::ProductInfo(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
: wxPanel(parent, id, position, size, style), mManager(this, ab::AuiTheme::AUIMGRSTYLE){
	SetupAuitheme();

	CreateChoices();
	CreateMainTool();
	CreateInventoryView();
	CreateProperyGrid();
	CreateHistoryView();
	mManager.Update();
}

ab::ProductInfo::~ProductInfo()
{
	mInventoryModel.release();
	mHistModel.release();
}

void ab::ProductInfo::SetupAuitheme()
{
	auto artProvider = mManager.GetArtProvider();
	ab::AuiTheme::Update(artProvider);
	ab::AuiTheme::Register(std::bind_front(&ab::ProductInfo::OnAuiThemeChange, this));
}

void ab::ProductInfo::OnAuiThemeChange()
{
	auto auiArt = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiArt);
	mManager.Update();
}

void ab::ProductInfo::CreateMainTool()
{
	mMainToolbar = new wxAuiToolBar(this, ID_MAINTOOL, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mMainToolbar->SetToolBitmapSize(wxSize(FromDIP(16), FromDIP(16)));
	mMainToolbar->AddTool(ID_TOOL_GO_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, wxSize(16, 16)));
	mMainToolbar->AddSeparator();
	mMainToolbar->AddSpacer(FromDIP(5));
	mNameLabel = new wxStaticText(mMainToolbar, wxID_ANY, "TEST", wxDefaultPosition, wxDefaultSize, 0);
	mNameLabel->SetFont(wxFontInfo().AntiAliased().Bold());
	mNameLabel->SetBackgroundColour(*wxWHITE);
	mNameLabelItem = mMainToolbar->AddControl(mNameLabel);


	mMainToolbar->AddStretchSpacer();
	mMainToolbar->AddTool(ID_TOOL_ADD_INVENTORY, "Add stock", wxArtProvider::GetBitmap("add", wxART_OTHER, wxSize(16, 16)), "Add stock to the product");
	mMainToolbar->AddSpacer(FromDIP(10));
	mMainToolbar->AddTool(ID_TOOL_SHOW_PRODUCT_INFO, "Product info", wxArtProvider::GetBitmap("settings", wxART_OTHER, wxSize(16, 16)), "Show product information");
	mMainToolbar->AddSpacer(FromDIP(10));
	mMainToolbar->AddTool(ID_ADD_BARCODE, "Add barcode", wxArtProvider::GetBitmap("add", wxART_OTHER, wxSize(16, 16)), "Add barcode to product");
	mMainToolbar->AddSpacer(FromDIP(10));
	mMainToolbar->AddTool(ID_SHOW_PRODUCT_SALE_HISTORY, "Product sale history", wxArtProvider::GetBitmap("", wxART_OTHER, wxSize(16, 16)), "Show the product sale history");
	mMainToolbar->Realize();
	mManager.AddPane(mMainToolbar, wxAuiPaneInfo().Name("BottomToolBar").ToolbarPane().Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().Row(2).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::ProductInfo::CreateInventoryView()
{
	auto& app = wxGetApp();
	mInventoryBook = new wxSimplebook(this, wxID_ANY);
	mInventoryPanel = new wxPanel(mInventoryBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bsz = new wxBoxSizer(wxVERTICAL);
	
	mInventoryToolbar = new wxAuiToolBar(mInventoryPanel, ID_INVEN_TOOLBAR, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mInventoryToolbar->SetMinSize(wxSize(FromDIP(-1), FromDIP(30)));
	mInventoryToolbar->SetBackgroundColour(*wxWHITE); //add to theme


	mInventoryToolbar->AddStretchSpacer();
	mInventoryToolbar->AddSeparator();
	auto today = pof::base::data::clock_t::now();

	mStartDatePicker = new wxDatePickerCtrl(mInventoryToolbar, ID_INVEN_START_DATE_PICKER, wxDateTime::Now(), wxDefaultPosition, FromDIP(wxSize(100, -1)), wxDP_DROPDOWN);
	mStopDatePicker = new wxDatePickerCtrl(mInventoryToolbar, ID_INVEN_STOP_DATE_PICKER, wxDateTime::Now(), wxDefaultPosition, FromDIP(wxSize(100, -1)), wxDP_DROPDOWN);

	mInventoryToolbar->AddControl(new wxStaticText(mInventoryToolbar, wxID_ANY, "Start:"), "Start date");
	mInventoryToolbar->AddSpacer(FromDIP(10));
	mInventoryToolbar->AddControl(mStartDatePicker);

	mInventoryToolbar->AddSpacer(FromDIP(10));
	mInventoryToolbar->AddControl(new wxStaticText(mInventoryToolbar, wxID_ANY, "Stop:"), "Stop date");
	mInventoryToolbar->AddSpacer(FromDIP(10));
	mInventoryToolbar->AddControl(mStopDatePicker);
	mInventoryToolbar->Realize();

	mInventoryView = new wxDataViewCtrl(mInventoryPanel, ID_DATA_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mInventoryModel = std::make_unique<ab::DataModel<grape::inventory>>();
	mInventoryView->AssociateModel(mInventoryModel.get());
	mInventoryModel->DecRef();

	//mInventoryView->AppendDateColumn(wxT("Input Date"), 5, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	//mInventoryView->AppendDateColumn(wxT("Expiry Date"), 4, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInventoryView->AppendTextColumn(wxT("Stock count"), 6, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mInventoryView->AppendTextColumn(wxT("Cost Price"), 7, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);

	ab::DataModel<grape::inventory>::specialcol_t col;
	col.first = [&](wxVariant& v, int row, int col) {
		auto& arr = mInventoryModel->GetRow(row);
		v = boost::lexical_cast<std::string>(arr[col].GetLongLong().GetValue());
	};
	mInventoryModel->AddSpecialCol(std::move(col), 6);

	bsz->Add(mInventoryToolbar, wxSizerFlags().Expand().Border(wxALL, FromDIP(2)));
	bsz->Add(mInventoryView, wxSizerFlags().Expand().Proportion(1).Border(wxALL, FromDIP(2)));

	mInventoryPanel->SetSizer(bsz);
	mInventoryPanel->Layout();

	std::tie(mInventoryWaitPanel, mInventoryActivity) = app.CreateWaitPanel(mInventoryBook, "Please wait...");
	wxButton* mAddStock = nullptr;
	std::tie(mInventoryEmptyPanel, std::ignore, mAddStock) = app.CreateEmptyPanel(mInventoryBook, "No inventory for product", "supplement-bottle");
	mAddStock->SetLabel("Add stock");
	mAddStock->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_BUTTON));
	mAddStock->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		OnAddStock(evt);
	});

	wxButton* mRetry = nullptr;
	std::tie(mInventoryErrorPanel, std::ignore, mRetry) = app.CreateEmptyPanel(mInventoryBook, "Server error", wxART_ERROR, wxSize(48,48), wxART_MESSAGE_BOX);
	mRetry->SetLabel("Retry");
	mRetry->SetBitmap(wxArtProvider::GetBitmap(wxART_REFRESH, wxART_BUTTON));
	mRetry->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		mInventoryBook->SetSelection(INVEN_WAIT);
		mInventoryActivity->Start();
		boost::asio::post(std::bind(&ab::ProductInfo::GetInventory, this, 0, 1000));
	});

	mInventoryBook->AddPage(mInventoryPanel, "Inventory", false);
	mInventoryBook->AddPage(mInventoryWaitPanel, "Inven wait", false);
	mInventoryBook->AddPage(mInventoryEmptyPanel, "Inven empty", false);
	mInventoryBook->AddPage(mInventoryErrorPanel, "Inven error", false);


	mManager.AddPane(mInventoryBook, wxAuiPaneInfo().Name("InventoryView").Caption("Sale History").CenterPane());

}

void ab::ProductInfo::CreateHistoryView()
{
	mHistBook = new wxSimplebook(this, wxID_ANY);
	mHistPanel = new wxPanel(mHistBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bsz = new wxBoxSizer(wxVERTICAL);



	mHistView = new wxDataViewCtrl(mHistPanel, ID_DATA_VIEW_HIST, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mHistModel = std::make_unique<ab::DataModel<grape::sale_history>>();
	mHistView->AssociateModel(mHistModel.get());
	mHistModel->DecRef();


	bsz->Add(mHistView, wxSizerFlags().Expand().Proportion(1).Border(wxALL, FromDIP(2)));
	mHistPanel->SetSizer(bsz);
	mHistPanel->Layout();

	mHistBook->AddPage(mHistPanel, "hist view", false);
	mManager.AddPane(mHistBook, wxAuiPaneInfo().Name("Hist").Caption("Product History").Hide());

}

void ab::ProductInfo::CreateProperyGrid()
{
	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(548, -1)));
	wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);

	mProductInfoGridManager = new wxPropertyGridManager(panel, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(548, -1)), wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER |
		wxPG_TOOLBAR |
		wxPG_DESCRIPTION |
		wxPG_SPLITTER_AUTO_CENTER |
		wxNO_BORDER |
		wxPGMAN_DEFAULT_STYLE);
	mProductInfoGridManager->SetExtraStyle(wxPG_EX_NATIVE_DOUBLE_BUFFERING | wxPG_EX_MODE_BUTTONS);

	mProductInfoPage = mProductInfoGridManager->AddPage("General", wxNullBitmap);
	mProductInfoPage->GetGrid()->SetSize(FromDIP(wxSize(548, -1)));

	mProductInfoPage->GetGrid()->SetBackgroundColour(*wxWHITE);
	mProductInfoPage->GetGrid()->SetCaptionBackgroundColour(wxTheColourDatabase->Find("Aqua"));
	mProductInfoPage->GetGrid()->SetCaptionTextColour(*wxBLACK);
	mProductInfoPage->GetGrid()->SetMarginColour(wxTheColourDatabase->Find("Aqua"));

	auto tool = mProductInfoGridManager->GetToolBar();
	if (tool) {
		tool->SetBackgroundColour(*wxWHITE);
		tool->SetSize(FromDIP(wxSize(-1, 30)));
		tool->Realize();

	}

	//set the grid up
	auto c0 = mProductInfoPage->Append(new wxPropertyCategory("Product details"));
	p[0] = mProductInfoPage->Append(new wxStringProperty("Name", "0", mSelectedProduct.name));
	p[1] = mProductInfoPage->Append(new wxArrayStringProperty("Generic Name", "1"));
	p[2] = mProductInfoPage->Append(new wxIntProperty("Package size", "2", mSelectedProduct.package_size));
	p[3] = mProductInfoPage->Append(new wxEnumProperty("Class", "3", ProductClassChoices, 0));
	p[4] = mProductInfoPage->Append(new wxStringProperty("Barcode", "4", mSelectedProduct.barcode));
	p[4]->Enable(false);
	p[5] = mProductInfoPage->Append(new wxEnumProperty("Formulation", "5", FormulationChoices, 0));

	auto c1  = mProductInfoPage->Append(new wxPropertyCategory("More Product details"));
	p[6]  = mProductInfoPage->Append(new wxArrayStringProperty("Usage information", "6"));
	p[7]  = mProductInfoPage->Append(new wxArrayStringProperty("Indications", "7"));
	p[8]  = mProductInfoPage->Append(new wxArrayStringProperty("Side effects", "8"));
	p[9]  = mProductInfoPage->Append(new wxStringProperty("Strength/conc", "9"));
	p[10] = mProductInfoPage->Append(new wxEnumProperty("Stength unit", "10"));

	auto c2 = mProductInfoPage->Append(new wxPropertyCategory("Sale"));
	wxFloatingPointValidator<double> val(2, &mStubPrice, wxNUM_VAL_ZERO_AS_BLANK);
	val.SetRange(0, 999999999999);
	p[11] = mProductInfoPage->Append(new wxFloatProperty("Unit price", "11"));
	p[12] = mProductInfoPage->Append(new wxFloatProperty("Cost price", "12"));
	p[11]->SetValidator(val);
	p[12]->SetValidator(val);
	p[13] = mProductInfoPage->Append(new wxIntProperty("Current stock", "13"));

	bs->Add(mProductInfoGridManager, wxSizerFlags().Expand().Proportion(1));
	panel->SetSizer(bs);
	panel->Layout();

	mManager.AddPane(panel, wxAuiPaneInfo().Name("ProductProperty")
		.Caption("Product property").CaptionVisible().Right().BestSize(FromDIP(wxSize(548, -1)))
		.PaneBorder().BottomDockable(false).Floatable(false).TopDockable(false).Show());
}

void ab::ProductInfo::CreateWarnings()
{
}

void ab::ProductInfo::CreateChoices()
{
	auto& app = wxGetApp();
	try {
		ProductClassChoices.Add("POM");
		ProductClassChoices.Add("OTC");
		ProductClassChoices.Add("CONTROLLED");
		ProductClassChoices.Add("NOT SPECIFIED");

		auto iter = app.settings.find("choices");
		if (iter == app.settings.end()) {
			//no choices load default
			FormulationChoices.Add("NOT SPECIFIED");
			FormulationChoices.Add("TABLET");
			FormulationChoices.Add("CAPSULE");
			FormulationChoices.Add("SOLUTION");
			FormulationChoices.Add("SUSPENSION");
			FormulationChoices.Add("SYRUP");
			FormulationChoices.Add("IV");
			FormulationChoices.Add("IM");
			FormulationChoices.Add("EMULSION");
			FormulationChoices.Add("COMSUMABLE");
			FormulationChoices.Add("POWDER");
			FormulationChoices.Add("OINTMNET");
			FormulationChoices.Add("EYE DROP");
			FormulationChoices.Add("SUPPOSITORY");
			FormulationChoices.Add("LOZENGES");
			FormulationChoices.Add("OIL");
			FormulationChoices.Add("NOT SPECIFIED");

			StrengthChoices.Add("g");
			StrengthChoices.Add("mg");
			StrengthChoices.Add("mcg");
			StrengthChoices.Add("L");
			StrengthChoices.Add("ml");
			StrengthChoices.Add("ml");
			StrengthChoices.Add("%v/v");
			StrengthChoices.Add("%w/v");
			StrengthChoices.Add("mol");
			StrengthChoices.Add("mmol");
			StrengthChoices.Add("NOT SPECIFIED");
		}
		else {
			auto& form = (*iter)["formchoices"];
			auto& strength = (*iter)["strength"];

			for (auto& f : form) {
				FormulationChoices.Add(static_cast<std::string>(f));
			}
			FormulationChoices.Add("NOT SPECIFIED");

			for (auto& s : strength) {
				StrengthChoices.Add(static_cast<std::string>(s));
			}
			StrengthChoices.Add("NOT SPECIFIED");
		}
	}
	catch (std::exception& exp) {
		spdlog::error(exp.what());
	}
}

void ab::ProductInfo::OnAddBarcode(wxCommandEvent& evt)
{
}

void ab::ProductInfo::OnCacheHint(wxDataViewEvent& evt)
{
}

void ab::ProductInfo::OnSave(wxCommandEvent& evt)
{
	//
}

void ab::ProductInfo::OnDateChange(wxDateEvent& evt)
{
}

void ab::ProductInfo::EnableByFormulary()
{
	//enable or disable because of the formulary

}

void ab::ProductInfo::GetInventory(size_t begin, size_t limit)
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

		grape::product_identifier pd;
		pd.product_id  = mSelectedProduct.id;
		pd.pharmacy_id = cred.pharm_id;
		pd.branch_id   = cred.branch_id;

		grape::page page;
		page.begin = begin;
		page.limit = limit;

		constexpr const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(page) + 
			 grape::serial::get_size(pd);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf  = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf, pd);
		auto buf3 = grape::serial::write(buf2, page);

		grape::body_type body2 = body;
		//get count
		auto fut = sess->req(http::verb::get, "/product/inventory/count"s, std::move(body));
		auto resp = fut.get();
		if(resp.result() == http::status::not_found)
		{
			mInventoryBook->SetSelection(INVEN_EMPTY);
			return;
		}
		else if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));
		auto& b = resp.body();
		if (b.empty()) throw std::logic_error("no data receievd");
		auto&& [count, rbuf] = grape::serial::read<boost::fusion::vector<std::int64_t>>(boost::asio::buffer(b));
		mInventoryCount = boost::fusion::at_c<0>(count);

		//load iventory
		fut = sess->req(http::verb::get, "/product/inventory/get", std::move(body2));
		resp = fut.get();

		if (resp.result() != http::status::ok) {
			//product cannot  exists without any formulary
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& b2 = resp.body();
		if (b2.empty())
			throw std::logic_error("expected a body, no body received");

		auto&& [col, rbuf2] = grape::serial::read<grape::collection_type<grape::inventory>> (boost::asio::buffer(b2));

		//also get the product formulary
		grape::uid_t pid;
		boost::fusion::at_c<0>(pid) = mSelectedProduct.id;

		constexpr size_t cs = grape::serial::get_size(cred) + grape::serial::get_size(pid);
		grape::session::request_type::body_type::value_type body3(cs, 0x00);
		buf  = grape::serial::write(boost::asio::buffer(body3), cred);
		buf2 = grape::serial::write(buf, pid);

		fut = sess->req(http::verb::get, "/product/formulary/getproductformularies", std::move(body3));
		resp = fut.get();

		if (resp.result() != http::status::ok){
			//product cannot  exists without any formulary
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& b3 = resp.body();
		if (b3.empty())
			throw std::logic_error("expected a body, no body received");

		auto&& [collect, rbuf3] = grape::serial::read<grape::collection_type<grape::formulary>>(boost::asio::buffer(b3));


		mInventoryActivity->Stop();
		mInventoryModel->Reload(boost::fusion::at_c<0>(col), begin, limit, mInventoryCount );
		mInventoryBook->SetSelection(INVEN_VIEW);

	}
	catch (const std::exception& exp) {
		spdlog::error(std::format("{} :{}"s, std::source_location::current(), exp.what()));
		wxMessageBox(exp.what(), "Product information", wxICON_ERROR | wxOK);
		mInventoryBook->SetSelection(INVEN_ERROR);
	}
}

void ab::ProductInfo::GetHistory(size_t begin, size_t limit)
{
}

void ab::ProductInfo::GetProductFormulary()
{
}

void ab::ProductInfo::Load()
{
	constexpr auto split = [](const std::string& str) -> wxArrayString {
		wxArrayString ret;
		for (const auto s : std::ranges::views::split(str, ",")) {
			ret.push_back(std::string(s.begin(), s.end()));
		}
		return ret;
	};

	constexpr auto choice_s = [](const std::string& str, const wxPGChoices& mChoices) -> int {
		auto strs = mChoices.GetLabels();
		int ret = 0;
		for (auto& s : strs) {
			if (boost::iequals(str, s.ToStdString()))
				return ret;
			ret++;
		}
		return (int)(strs.GetCount() - 1); //last entry should be "NOT SPECIFIED"
	};

	p[0]->SetValue(wxVariant(mSelectedProduct.name));
	p[1]->SetValue(wxVariant(split(mSelectedProduct.generic_name)));
	p[2]->SetValue(wxVariant(wxLongLong(mSelectedProduct.package_size)));
	p[3]->SetValue(wxVariant(choice_s(mSelectedProduct.cls, ProductClassChoices)));
	p[4]->SetValue(wxVariant(mSelectedProduct.barcode));
	p[5]->SetValue(wxVariant(choice_s(mSelectedProduct.formulation, FormulationChoices)));
	p[6]->SetValue(wxVariant(split(mSelectedProduct.usage_info)));
	p[7]->SetValue(wxVariant(split(mSelectedProduct.indications)));
	p[8]->SetValue(wxVariant(split(mSelectedProduct.sideeffects)));
	p[9]->SetValue(wxVariant(mSelectedProduct.strength));
	p[10]->SetValue(wxVariant(choice_s(mSelectedProduct.strength_type, StrengthChoices)));
	p[11]->SetValue(wxVariant(static_cast<double>(mSelectedProduct.unit_price)));
	p[12]->SetValue(wxVariant(static_cast<double>(mSelectedProduct.cost_price)));
	p[13]->SetValue(wxVariant(wxLongLong(mSelectedProduct.stock_count)));

	//set name
	mMainToolbar->Freeze();
	mNameLabel->SetLabelText(mSelectedProduct.name);
	mNameLabelItem->SetMinSize(mNameLabel->GetSize());
	mMainToolbar->Realize();
	mMainToolbar->Thaw();

	mManager.Update();

	//get the inventory data from grape
	mInventoryBook->SetSelection(INVEN_WAIT);
	mInventoryActivity->Start();
	boost::asio::post(wxGetApp().mTaskManager.tp(),
		std::bind(&ab::ProductInfo::GetInventory, this, 0, 1000));
}

void ab::ProductInfo::UnLoad()
{
	mInventoryModel->Clear();
	mHistModel->Clear();
}

void ab::ProductInfo::OnPropertyChanging(wxPropertyGridEvent& evt)
{
	if (mProductFormulary.id.is_nil())
	{
		evt.Veto();
		return;
	}
}

void ab::ProductInfo::OnPropertyChanged(wxPropertyGridEvent& evt)
{
	auto p = evt.GetProperty();
	if (!p) return;

	size_t n = boost::lexical_cast<size_t>(p->GetName().ToStdString());
	switch (n)
	{
	case 0: //name
		break;
	case 1: //generic name
		break;
	case 2: // package size
		break;
	case 3: //class
		break;
	case 4: // barcode
		break;
	case 5: // formulation
		break;
	case 6: //usage info
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	case 11:
		break;
	case 12:
		break;
	case 13:
		break;
	default:
		break;
	}
}

void ab::ProductInfo::OnBack(wxCommandEvent& evt)
{
	mOnBack();
}

void ab::ProductInfo::OnHistory(wxCommandEvent& evt)
{
	auto& paneInfo = mManager.GetPane("Hist");
	if (!paneInfo.IsOk()) return;
	paneInfo.Show(!paneInfo.IsShown());
	mManager.Update();
}

void ab::ProductInfo::OnProductProperty(wxCommandEvent& evt)
{
	auto& paneInfo = mManager.GetPane("ProductProperty");
	if (!paneInfo.IsOk()) return;
	paneInfo.Show(!paneInfo.IsShown());
	mManager.Update();
}

void ab::ProductInfo::OnAddStock(wxCommandEvent& evt)
{
	auto& app = wxGetApp();
	wxDialog dialog(this, wxID_ANY, "Add stock");
	auto d = std::addressof(dialog);
	d->SetSize(FromDIP(wxSize(591, 398)));
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

	wxStaticText* Title = new wxStaticText(d, wxID_ANY, "Add Stock");
	Title->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS).Bold()));
	wxStaticText* Description = new wxStaticText(d, wxID_ANY, "Adds stock to the product for sale");


	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Batch No"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxTextCtrl* mBatchNumber = new wxTextCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)));
	mBatchNumber->SetValidator(wxTextValidator{ wxFILTER_DIGITS | wxFILTER_EMPTY });
	flexSizer->Add(mBatchNumber, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();

	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Quantity"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxSpinCtrl* mQuantityInControl = new wxSpinCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(200, -1)), wxSP_ARROW_KEYS | wxALIGN_LEFT, 0,	std::numeric_limits<int>::max());
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

	wxCheckBox* mAddSupplier = new wxCheckBox(d, wxID_ANY, "Add supplier", wxDefaultPosition, wxDefaultSize);
	flexSizer->Add(mAddSupplier, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
	flexSizer->AddStretchSpacer();
	flexSizer->AddStretchSpacer();

	flexSizer->Add(new wxStaticText(d, wxID_ANY, "Invoice No"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	wxTextCtrl* mInvoiceName = new wxTextCtrl(d, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	flexSizer->Add(mInvoiceName, wxSizerFlags().Expand().Align(wxLEFT).Border(wxALL, FromDIP(5)));
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
		grape::inventory inven; // = { 0 };
		inven.pharmacy_id = app.mPharmacyManager.pharmacy.id;
		inven.branch_id   = app.mPharmacyManager.branch.id;
		inven.product_id  = mSelectedProduct.id;
		inven.input_date  = std::chrono::system_clock::now();
		inven.cost        = pof::base::currency(boost::lexical_cast<float>(mCostControl->GetValue()));
		inven.expire_date = std::chrono::system_clock::from_time_t(mExpiryDate->GetValue().GetTicks());
		inven.lot_number  = mBatchNumber->GetValue().ToStdString();
		inven.stock_count = static_cast<std::uint64_t>(mQuantityInControl->GetValue());

		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		grape::credentials cred; //= { 0 };
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();

		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(inven);
		grape::body_type body(size, 0x00);
		auto wbuf = grape::serial::write(boost::asio::buffer(body), cred);
		auto wbuf2 = grape::serial::write(wbuf, inven);

		auto fut = sess->req(http::verb::post, "/product/inventory/add", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Adding stock to product...");
			resp = fut.get();
		}
		if (resp.result() != http::status::ok)
			throw std::logic_error(app.ParseServerError(resp));

		//everything is good refresh the inventory view
			//get the inventory data from grape
		mInventoryBook->SetSelection(INVEN_WAIT);
		mInventoryActivity->Start();
		boost::asio::post(wxGetApp().mTaskManager.tp(),
			std::bind(&ab::ProductInfo::GetInventory, this, 0, 1000));
		//overhead cost
		//std::thread{ std::bind_front(&ab::ProductInfo::GetInventory, this), 0, 1000 }.detach();
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(exp.what(), "Add stock", wxICON_ERROR | wxOK);
	}
}
