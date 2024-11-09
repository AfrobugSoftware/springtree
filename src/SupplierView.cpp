#include "SupplierView.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::SupplierView, wxPanel)
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
	mManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").CenterPane());
	mManager.Update();
}

void ab::SupplierView::OnAddSupplier(wxCommandEvent& evt)
{
}

void ab::SupplierView::OnAddInvoice(wxCommandEvent& evt)
{
}

void ab::SupplierView::OnInvoiceContextMenu(wxDataViewEvent& evt)
{
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

	mTools->AddSpacer(5);
	mSupplierSearch = new wxSearchCtrl(mTools, ID_SEARCH, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(400, -1)), wxWANTS_CHARS);
	mTools->AddControl(mSupplierSearch);

	mTools->AddStretchSpacer();
	mTools->AddSeparator();
	mTools->AddTool(ID_ADD_SUPPLIER, "Create supplier", wxArtProvider::GetBitmap("add", wxART_OTHER, FromDIP(wxSize(16, 16))), "Create supplier");
	mTools->AddSpacer(FromDIP(5));
	mTools->Realize();
	mManager.AddPane(mTools, wxAuiPaneInfo().Name("Tools").Top().MinSize(FromDIP(wxSize(-1, 30))).PaneBorder(false).ToolbarPane().Top().DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));

	mInvoiceTools = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mInvoiceTools->SetToolBitmapSize(FromDIP(wxSize(16, 16)));
	mInvoiceTools->AddTool(ID_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16, 16))), "Back");

	mSupplierName = new wxStaticText(mInvoiceTools, wxID_ANY, "TEST", wxDefaultPosition, wxDefaultSize, 0);
	mSupplierName->SetFont(wxFontInfo().AntiAliased().Bold());
	mSupplierName->SetBackgroundColour(*wxWHITE);

	mInvoiceTools->AddSeparator();
	mInvoiceTools->AddSpacer(FromDIP(5));
	mSupplierNameItem = mInvoiceTools->AddControl(mSupplierName);

	mInvoiceTools->AddStretchSpacer();
	mCreateInvoiceItem = mInvoiceTools->AddTool(ID_CREATE_INVOICE, "Add invoice", wxArtProvider::GetBitmap("add", wxART_OTHER, FromDIP(wxSize(16, 16))), "Add a new invoice");
	mInvoiceTools->Realize();
	mManager.AddPane(mInvoiceTools, wxAuiPaneInfo().Name("InvoiceTools").Top().MinSize(FromDIP(wxSize(-1, 30))).PaneBorder(false).ToolbarPane().Top().DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false).Hide());

	mInvoiceProductTools = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_NO_AUTORESIZE | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mInvoiceProductTools->SetToolBitmapSize(FromDIP(wxSize(16, 16)));
	mInvoiceProductSearch = new wxSearchCtrl(mInvoiceProductTools, ID_PRODUCT_SEARCH, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(400, -1)), wxWANTS_CHARS);
	mInvoiceProductTools->AddSpacer(FromDIP(5));
	mInvoiceProductTools->AddControl(mInvoiceProductSearch);

	mInvoiceProductTools->Realize();
	mManager.AddPane(mInvoiceProductTools, wxAuiPaneInfo().Name("InvoiceProductTools").Top().MinSize(FromDIP(wxSize(-1, 30))).PaneBorder(false).ToolbarPane().Top().DockFixed().Row(1).LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false).Hide());
}

void ab::SupplierView::CreatePanels()
{
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
	mInvoiceProductModel->DecRef();

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
