#include "SaleView.hpp"
#include "Application.hpp"


BEGIN_EVENT_TABLE(ab::SaleView, wxPanel)
	EVT_BUTTON(ab::SaleView::ID_CHECKOUT, ab::SaleView::OnCheckOut)
	EVT_BUTTON(ab::SaleView::ID_SAVE, ab::SaleView::OnSave)
	EVT_BUTTON(ab::SaleView::ID_CLEAR, ab::SaleView::OnClear)
	EVT_TOOL(ab::SaleView::ID_NEW_SALE, ab::SaleView::OnNewSale)
END_EVENT_TABLE()


ab::SaleView::SaleView(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	: wxPanel(parent, id, position, size, style), mManager(this, ab::AuiTheme::AUIMGRSTYLE) {
	SetDoubleBuffered(true);
	SetupAuiTheme();
	CreateToolbar();
	CreateMainPane();

	mManager.Update();
}

ab::SaleView::~SaleView()
{
}

void ab::SaleView::SetupAuiTheme()
{
	auto auiart = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
	ab::AuiTheme::Register(std::bind_front(&ab::SaleView::OnAuiThemeChange, this));
}

void ab::SaleView::OnAuiThemeChange()
{
	auto auiart = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
}

void ab::SaleView::CreateToolbar()
{
	mTopTools = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mTopTools->SetMinSize(FromDIP(wxSize(-1, 30)));

	mBottomTools = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mBottomTools->SetMinSize(FromDIP(wxSize(-1, 30)));

	mProductNameValue = new wxSearchCtrl(mTopTools, ID_PRODUCT_SEARCH_NAME, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(250, -1)));
	mProductNameValue->SetHint("Search product by name");
	mProductNameValue->ShowCancelButton(true);
	mTopTools->AddControl(mProductNameValue);

	mTopTools->AddSpacer(FromDIP(10));

	mScanProductValue = new wxSearchCtrl(mTopTools, ID_PRODUCT_SCAN, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(250, -1)), 0);
	mScanProductValue->SetValidator(wxTextValidator{ wxFILTER_DIGITS });
	mScanProductValue->SetHint("Scan products");
	mScanProductValue->ShowCancelButton(true);

	mTopTools->AddControl(mScanProductValue);

	mTopTools->AddStretchSpacer();

	mTopTools->AddTool(ID_NEW_SALE, wxT("New Sale"), wxArtProvider::GetBitmap("create", wxART_OTHER, FromDIP(wxSize(16, 16))), "Start a new sale");
	mTopTools->AddTool(ID_PACKS, wxT("Rx Packs"), wxArtProvider::GetBitmap("shopping_bag", wxART_OTHER, FromDIP(wxSize(16, 16))), "Product packs");
	//mTopTools->AddTool(ID_PRINT_LABELS, wxT("Print As Labels"), wxArtProvider::GetBitmap("download_down", wxART_OTHER, FromDIP(wxSize(16, 16))), "Print labels of product sold");
	mTopTools->AddTool(ID_REMOVE_PRODUCT, wxT("Remove Product"), wxArtProvider::GetBitmap("delete", wxART_OTHER, FromDIP(wxSize(16, 16))), "Remove product from sale list");
	mTopTools->AddTool(ID_HIDE_PRODUCT_VIEW_PROPERTY, wxT("View Product"), wxArtProvider::GetBitmap("edit_note", wxART_OTHER, FromDIP(wxSize(16, 16))), "View details about the product");


	mBottomTools->AddTool(ID_OPEN_SAVE_SALE, wxT("Saved Sales"), wxArtProvider::GetBitmap("shopping_cart", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mBottomTools->AddSpacer(FromDIP(5));
	mReprintItem = mBottomTools->AddTool(ID_REPRINT, "Reprint", wxArtProvider::GetBitmap("print", wxART_OTHER, FromDIP(wxSize(16, 16))), "Reprint a sale");
	mReprintItem->SetHasDropDown(true);
	mBottomTools->AddSpacer(FromDIP(5));
	mReturnItem = mBottomTools->AddTool(ID_RETURN_SALE, "Return", wxArtProvider::GetBitmap("redo", wxART_OTHER, FromDIP(wxSize(16, 16))), "Return an item");
	mBottomTools->AddSpacer(FromDIP(5));
	mBottomTools->AddTool(ID_DISCOUNT, "Add discount", wxArtProvider::GetBitmap("add_task", wxART_OTHER, FromDIP(wxSize(16, 16))), "Add discount to an item");
	mReturnItem->SetHasDropDown(true);
	mBottomTools->AddSpacer(FromDIP(5));

	//look for how to make this more dynamic
	auto pt = new wxStaticText(mBottomTools, wxID_ANY, "Payment option:");
	pt->SetBackgroundColour(*wxWHITE);
	mBottomTools->AddControl(pt);
	mBottomTools->AddSpacer(FromDIP(5));

	paymentTypes.push_back("Cash");
	paymentTypes.push_back("Transfer");
	paymentTypes.push_back("POS");
	paymentTypes.push_back("No payment option");

	mPaymentTypes = new wxChoice(mBottomTools, ID_PAYMENT_TYPE, wxDefaultPosition, FromDIP(wxSize(150, 20)), paymentTypes);
	mPaymentTypes->SetSelection(paymentTypes.size() - 1);
	mPaymentTypes->Bind(wxEVT_PAINT, [=](wxPaintEvent& evt) {
		wxPaintDC dc(mPaymentTypes);
		wxRect rect(0, 0, dc.GetSize().GetWidth(), dc.GetSize().GetHeight());


		dc.SetBrush(*wxWHITE);
		dc.SetPen(*wxGREY_PEN);
		dc.DrawRoundedRectangle(rect, 2.0f);
		dc.DrawBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_OTHER, FromDIP(wxSize(10, 10))), wxPoint(rect.GetWidth() - FromDIP(15), (rect.GetHeight() / 2) - FromDIP(5)));
		auto sel = mPaymentTypes->GetStringSelection();
		if (!sel.IsEmpty()) {
			dc.DrawLabel(sel, rect, wxALIGN_CENTER);
		}
		});
	mBottomTools->AddControl(mPaymentTypes, "Payment type");
	mBottomTools->AddStretchSpacer();

	mBottomTools->AddSpacer(5);
	mActiveSaleId = new wxStaticText(mBottomTools, ID_ACTIVE_UI_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	mActiveSaleId->SetBackgroundColour(*wxWHITE);
	mActiveSaleId->SetDoubleBuffered(true);
	mActiveSaleTextItem = mBottomTools->AddControl(mActiveSaleId);

	mTopTools->Realize();
	mBottomTools->Realize();
	mManager.AddPane(mTopTools, wxAuiPaneInfo().Name("TopTools").ToolbarPane().Top().Row(1).MinSize(FromDIP(-1), FromDIP(30)).DockFixed().LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
	mManager.AddPane(mBottomTools, wxAuiPaneInfo().Name("BottomTools").ToolbarPane().Row(2).Top().MinSize(FromDIP(-1), FromDIP(30)).DockFixed().LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));

}

void ab::SaleView::CreateView()
{
	if (mSaleView.size() >= max_view){
		wxMessageBox("Cannot open any more views, close some sales",
			"Sales", wxICON_WARNING | wxOK);
		return;
	}
	auto view = 
		mSaleView.emplace_back(new wxDataViewCtrl(mSaleNotebook, ID_SALE_VIEW + mSaleView.size(), wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES));
	auto mod = new ab::DataModel<grape::sale_display>();
	view->AssociateModel(mod);
	mod->DecRef();

	view->AppendTextColumn(wxT("Product"),      0, wxDATAVIEW_CELL_INERT,    FromDIP(250), wxALIGN_CENTER);
	view->AppendTextColumn(wxT("Quantity"),     1, wxDATAVIEW_CELL_EDITABLE, FromDIP(100), wxALIGN_CENTER);
	view->AppendTextColumn(wxT("Price"),        2, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_CENTER);
	view->AppendTextColumn(wxT("Discount"),     3, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_CENTER);
	view->AppendTextColumn(wxT("Extact Price"), 4, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_CENTER);
	
	mSaleNotebook->AddPage(view, "DataPane", false);
}

void ab::SaleView::CreateMainPane()
{
	mDataPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizer;
	sizer = new wxBoxSizer(wxVERTICAL);

	mInfoBar = new wxInfoBar(mDataPane, wxID_ANY);
	mBook    = new wxSimplebook(mDataPane, wxID_ANY);

	mSaleNotebook = new wxAuiNotebook(mBook, ID_SALE_BOOK, wxDefaultPosition, wxDefaultSize);
	wxButton* packButton;
	std::tie(mEmpty, std::ignore,packButton) =
		wxGetApp().CreateEmptyPanel(mBook, "Add product to begin sale",
			"checkout_big");
	packButton->SetLabel("Open packs");
	packButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		wxMessageBox("Testing");
	});

	mBook->AddPage(mSaleNotebook, "SaleBook", false);
	mBook->AddPage(mEmpty, "Emppty", true);

	mSaleOutputPane = new wxPanel(mDataPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	mSaleOutputPane->SetDoubleBuffered(true);
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	mSaleDisplayPane = new wxPanel(mSaleOutputPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDOUBLE_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);


	bSizer3->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	mTextOutPut = new wxPanel(mSaleDisplayPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer(4, 2, 5, 5);

	const int fontSize = FromDIP(9);
	const int valueFontSize = FromDIP(10);
	wxFont valueFont(wxFontInfo(valueFontSize).AntiAliased());
	mQuantity = new wxStaticText(mTextOutPut, wxID_ANY, wxT("Quantity"), wxDefaultPosition, wxDefaultSize, 0);
	mQuantity->Wrap(-1);
	mQuantity->SetFont(wxFont(wxFontInfo(fontSize).Bold().AntiAliased()));
	gSizer1->Add(mQuantity, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mQuantityValue = new wxStaticText(mTextOutPut, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0);
	mQuantityValue->Wrap(-1);
	mQuantityValue->SetFont(valueFont);
	gSizer1->Add(mQuantityValue, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mExtQuantity = new wxStaticText(mTextOutPut, wxID_ANY, wxT("Ext. Quantity"), wxDefaultPosition, wxDefaultSize, 0);
	mExtQuantity->Wrap(-1);
	mExtQuantity->SetFont(wxFont(wxFontInfo(fontSize).Bold().AntiAliased()));
	gSizer1->Add(mExtQuantity, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mExtQuantityItem = new wxStaticText(mTextOutPut, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0);
	mExtQuantityItem->Wrap(-1);
	mExtQuantityItem->SetFont(valueFont);
	gSizer1->Add(mExtQuantityItem, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mDiscountAmount = new wxStaticText(mTextOutPut, wxID_ANY, wxT("Discount"), wxDefaultPosition, wxDefaultSize, 0);
	mDiscountAmount->Wrap(-1);
	mDiscountAmount->SetFont(wxFont(wxFontInfo(fontSize).Bold().AntiAliased()));
	gSizer1->Add(mDiscountAmount, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mDiscountValue = new wxStaticText(mTextOutPut, wxID_ANY, fmt::format("{:cu}", pof::base::currency{}), wxDefaultPosition, wxDefaultSize, 0);
	mDiscountValue->Wrap(-1);
	mDiscountValue->SetFont(valueFont);
	gSizer1->Add(mDiscountValue, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mTotalQuantity = new wxStaticText(mTextOutPut, wxID_ANY, wxT("Total Quantity"), wxDefaultPosition, wxDefaultSize, 0);
	mTotalQuantity->Wrap(-1);
	mTotalQuantity->SetFont(wxFont(wxFontInfo(fontSize).Bold().AntiAliased()));
	gSizer1->Add(mTotalQuantity, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));

	mTotalQuantityValue = new wxStaticText(mTextOutPut, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, 0);
	mTotalQuantityValue->Wrap(-1);
	mTotalQuantityValue->SetFont(valueFont);
	gSizer1->Add(mTotalQuantityValue, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));


	mTextOutPut->SetSizer(gSizer1);
	mTextOutPut->Layout();
	gSizer1->Fit(mTextOutPut);
	bSizer3->Add(mTextOutPut, 0, wxEXPAND | wxALL, FromDIP(5));


	mSaleDisplayPane->SetSizer(bSizer3);
	mSaleDisplayPane->Layout();
	bSizer3->Fit(mSaleDisplayPane);
	bSizer2->Add(mSaleDisplayPane, 1, wxEXPAND | wxALL, FromDIP(5));

	mSalePaymentButtonsPane = new wxPanel(mSaleOutputPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDOUBLE_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);


	mClear = new wxButton(mSalePaymentButtonsPane, ID_CLEAR, wxT("CLEAR"), wxDefaultPosition, FromDIP(wxSize(120, 50)), wxSIMPLE_BORDER);
	mClear->SetBackgroundColour(*wxWHITE);
	bSizer4->Add(mClear, 0, wxALL, FromDIP(5));

	mSave = new wxButton(mSalePaymentButtonsPane, wxID_SAVE, wxT("SAVE"), wxDefaultPosition, FromDIP(wxSize(120, 50)), wxSIMPLE_BORDER);
	mSave->SetBackgroundColour(*wxWHITE);
	bSizer4->Add(mSave, 0, wxALL, FromDIP(5));

	mCheckout = new wxButton(mSalePaymentButtonsPane, ID_CHECKOUT, wxT("CHECK OUT"), wxDefaultPosition, FromDIP(wxSize(120, 50)), wxSIMPLE_BORDER);
	mCheckout->SetBackgroundColour(*wxWHITE);
	bSizer4->Add(mCheckout, 0, wxALL, FromDIP(5));

	bSizer4->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	mTotalAmountLabel = new wxStaticText(mSalePaymentButtonsPane, wxID_ANY, wxT("TOTAL AMOUNT: "), wxDefaultPosition, wxDefaultSize, 0);
	mTotalAmountLabel->SetFont(wxFont(wxFontInfo(15).Bold().AntiAliased()));
	bSizer4->Add(mTotalAmountLabel, 0, wxALIGN_BOTTOM | wxALL, FromDIP(5));

	mTotalAmount = new wxStaticText(mSalePaymentButtonsPane, wxID_ANY, fmt::format("{:cu}", pof::base::currency{}), wxDefaultPosition, wxDefaultSize, 0);
	mTotalAmount->SetFont(wxFont(wxFontInfo(15).Bold().AntiAliased()));
	bSizer4->Add(mTotalAmount, 0, wxALIGN_BOTTOM | wxALL, FromDIP(5));

	mSalePaymentButtonsPane->SetSizer(bSizer4);
	mSalePaymentButtonsPane->Layout();
	bSizer4->Fit(mSalePaymentButtonsPane);
	bSizer2->Add(mSalePaymentButtonsPane, 0, wxEXPAND | wxALL, FromDIP(5));


	mSaleOutputPane->SetSizer(bSizer2);
	mSaleOutputPane->Layout();
	bSizer2->Fit(mSaleOutputPane);

	

	sizer->Add(mInfoBar, wxSizerFlags().Expand().Border(wxALL, FromDIP(5)));
	sizer->Add(mBook, wxSizerFlags().Expand().Proportion(1).Border(wxALL, FromDIP(5)));
	sizer->Add(mSaleOutputPane, wxSizerFlags().Expand().Border(wxALL, FromDIP(5)));
	mDataPane->SetSizer(sizer);
	mDataPane->Layout();

	mManager.AddPane(mDataPane, wxAuiPaneInfo().Name("DataPane").CenterPane().Show());
}

void ab::SaleView::OnCheckOut(wxCommandEvent& evt)
{

}

void ab::SaleView::OnClear(wxCommandEvent& evt)
{

}

void ab::SaleView::OnSave(wxCommandEvent& evt)
{

}

void ab::SaleView::OnNewSale(wxCommandEvent& evt)
{

}

void ab::SaleView::OnSaleNotebookClosed(wxAuiNotebookEvent& evt)
{

}
