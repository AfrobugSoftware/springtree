#include "ProductInfo.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::ProductInfo, wxPanel)
	EVT_TOOL(ab::ProductInfo::ID_TOOL_ADD_INVENTORY, ab::ProductInfo::OnAddStock)
	EVT_TOOL(ab::ProductInfo::ID_ADD_BARCODE, ab::ProductInfo::OnAddBarcode)
	EVT_TOOL(ab::ProductInfo::ID_SHOW_PRODUCT_SALE_HISTORY, ab::ProductInfo::OnHistory)
	EVT_TOOL(ab::ProductInfo::ID_TOOL_SHOW_PRODUCT_INFO, ab::ProductInfo::OnProductProperty)
	EVT_PG_CHANGING(ab::ProductInfo::ID_PROPERTY_GRID, ab::ProductInfo::OnPropertyChanging)
	EVT_DATE_CHANGED(ab::ProductInfo::ID_INVEN_START_DATE_PICKER, ab::ProductInfo::OnDateChange)
	EVT_DATE_CHANGED(ab::ProductInfo::ID_INVEN_STOP_DATE_PICKER, ab::ProductInfo::OnDateChange)
	EVT_DATAVIEW_CACHE_HINT(ab::ProductInfo::ID_DATA_VIEW, ab::ProductInfo::OnCacheHint)
END_EVENT_TABLE()


ab::ProductInfo::ProductInfo(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
: wxPanel(parent, id, position, size, style), mManager(this){
	SetupAuitheme();


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

	bsz->Add(mInventoryToolbar, wxSizerFlags().Expand().Border(wxALL, FromDIP(2)));
	bsz->Add(mInventoryView, wxSizerFlags().Expand().Proportion(1).Border(wxALL, FromDIP(2)));

	mInventoryPanel->SetSizer(bsz);
	mInventoryPanel->Layout();

	mInventoryBook->AddPage(mInventoryPanel, "Inventory", false);
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
	mManager.AddPane(mHistBook, wxAuiPaneInfo().Name("Histbook").Caption("Product History").Hide());

}

void ab::ProductInfo::CreateProperyGrid()
{
	mProductInfoGridManager = new wxPropertyGridManager(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxPG_BOLD_MODIFIED | wxPG_SPLITTER_AUTO_CENTER |
		wxPG_TOOLBAR |
		wxPG_DESCRIPTION |
		wxPG_SPLITTER_AUTO_CENTER |
		wxNO_BORDER |
		wxPGMAN_DEFAULT_STYLE);
	mProductInfoGridManager->SetExtraStyle(wxPG_EX_NATIVE_DOUBLE_BUFFERING | wxPG_EX_MODE_BUTTONS);
	mProductInfoPage = mProductInfoGridManager->AddPage("General", wxNullBitmap);
	mProductInfoPage->GetGrid()->SetBackgroundColour(*wxWHITE);
	mProductInfoPage->GetGrid()->SetCaptionBackgroundColour(wxTheColourDatabase->Find("Aqua"));
	mProductInfoPage->GetGrid()->SetCaptionTextColour(*wxBLACK);
	mProductInfoPage->GetGrid()->SetMarginColour(wxTheColourDatabase->Find("Aqua"));

	auto tool = mProductInfoGridManager->GetToolBar();
	if (tool) {
		tool->SetBackgroundColour(*wxWHITE);
	}

	//set the grid up
	auto c0 = mProductInfoPage->Append(new wxPropertyCategory("Product details"));
	p[0] = mProductInfoPage->Append(new wxStringProperty("Name", "0", mSelectedProduct.name));
	p[1] = mProductInfoPage->Append(new wxArrayStringProperty("Generic Name", "1"));
	p[2] = mProductInfoPage->Append(new wxIntProperty("Package size", "2", mSelectedProduct.package_size));
	p[3] = mProductInfoPage->Append(new wxEnumProperty("Class", "3", ProductClassChoices, 0));
	p[4] = mProductInfoPage->Append(new wxStringProperty("Barcode", "4", mSelectedProduct.barcode));
	p[5] = mProductInfoPage->Append(new wxEnumProperty("Formulation", "5", FormulationChoices, 0));

	auto c1  = mProductInfoPage->Append(new wxPropertyCategory("More Product details"));
	p[6]  = mProductInfoPage->Append(new wxArrayStringProperty("Usage information", "6"));
	p[7]  = mProductInfoPage->Append(new wxArrayStringProperty("Indications", "7"));
	p[8]  = mProductInfoPage->Append(new wxArrayStringProperty("Side effects", "8"));
	p[9]  = mProductInfoPage->Append(new wxStringProperty("Strength/conc", "9"));
	p[10] = mProductInfoPage->Append(new wxArrayStringProperty("Stength unit", "10"));

	auto c2 = mProductInfoPage->Append(new wxPropertyCategory("Sale"));
	wxFloatingPointValidator<double> val(2, &mStubPrice, wxNUM_VAL_ZERO_AS_BLANK);
	val.SetRange(0, 999999999999);
	p[11] = mProductInfoPage->Append(new wxFloatProperty("Unit price", "11"));
	p[12] = mProductInfoPage->Append(new wxFloatProperty("Cost price", "12"));
	p[11]->SetValidator(val);
	p[12]->SetValidator(val);
	p[13] = mProductInfoPage->Append(new wxIntProperty("Current stock", "13"));

	mManager.AddPane(mProductInfoGridManager, wxAuiPaneInfo().Name("ProductProperty").Caption("Product property").CaptionVisible().Left().Show());
}

void ab::ProductInfo::CreateWarnings()
{
}

void ab::ProductInfo::OnAddBarcode(wxCommandEvent& evt)
{
}

void ab::ProductInfo::OnCacheHint(wxDataViewEvent& evt)
{
}

void ab::ProductInfo::OnDateChange(wxDateEvent& evt)
{
}

void ab::ProductInfo::GetInventory(size_t begin, size_t limit)
{
}

void ab::ProductInfo::GetHistory(size_t begin, size_t limit)
{
}

void ab::ProductInfo::GetProductFormulary()
{
}

void ab::ProductInfo::OnPropertyChanging(wxPropertyGridEvent& evt)
{
}

void ab::ProductInfo::OnHistory(wxCommandEvent& evt)
{
}

void ab::ProductInfo::OnProductProperty(wxCommandEvent& evt)
{
}

void ab::ProductInfo::OnAddStock(wxCommandEvent& evt)
{
}
