#include "SaleView.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::SaleView, wxPanel)
	EVT_BUTTON(ab::SaleView::ID_CHECKOUT, ab::SaleView::OnCheckOut)
	EVT_BUTTON(ab::SaleView::ID_SAVE, ab::SaleView::OnSave)
	EVT_BUTTON(ab::SaleView::ID_CLEAR, ab::SaleView::OnClear)
	EVT_TOOL(ab::SaleView::ID_NEW_SALE, ab::SaleView::OnNewSale)
	EVT_TOOL(ab::SaleView::ID_PACKS, ab::SaleView::OnOpenPacks)

	EVT_SEARCH(ab::SaleView::ID_PRODUCT_SEARCH_NAME, ab::SaleView::OnProductSearch)
	EVT_TEXT(ab::SaleView::ID_PRODUCT_SEARCH_NAME, ab::SaleView::OnProductSearch)
	EVT_SEARCH_CANCEL(ab::SaleView::ID_PRODUCT_SEARCH_NAME, ab::SaleView::OnProductSearchCleared)
	EVT_AUINOTEBOOK_PAGE_CLOSE(ab::SaleView::ID_SALE_BOOK, ab::SaleView::OnSaleNotebookClosing)
	EVT_AUINOTEBOOK_PAGE_CLOSED(ab::SaleView::ID_SALE_BOOK, ab::SaleView::OnSaleNotebookClosed)
	EVT_AUINOTEBOOK_PAGE_CHANGED(ab::SaleView::ID_SALE_BOOK, ab::SaleView::OnSaleNotebookChanged)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(ab::SearchPopup, wxPopupTransientWindow)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::SearchPopup::ID_DATA_VIEW, ab::SearchPopup::OnDataItemSelected)
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
	mProductNameValue->Bind(wxEVT_CHAR, [&](wxKeyEvent& evt) {
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
	
	mSaleNotebook->AddPage(view, fmt::format("Sales - {:d}", mSaleView.size()), true, 0);
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
		OnOpenPacks(evt);
	});
	wxImageList* imgList = new wxImageList(16, 16);
	imgList->Add(wxArtProvider::GetBitmap("shopping_bag", wxART_OTHER, mSaleNotebook->FromDIP(wxSize(16, 16))));
	mSaleNotebook->AssignImageList(imgList);

	mBook->AddPage(mSaleNotebook, "SaleBook", false);
	mBook->AddPage(mEmpty, "Emppty", true);

	mSaleOutputPane = new wxPanel(mDataPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	mSaleOutputPane->SetDoubleBuffered(true);
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	mSaleDisplayPane = new wxPanel(mSaleOutputPane, ID_PAY_VIEW, wxDefaultPosition, wxDefaultSize, wxDOUBLE_BORDER | wxTAB_TRAVERSAL);
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

	mSearchPopup = new ab::SearchPopup(this);
	mSearchPopup->sSelectedSignal.connect(std::bind_front(&ab::SaleView::OnSearchedProduct, this));

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
		CreateView();
		if(mBook->GetSelection() != SALE_NOTEBOOK)
			mBook->SetSelection(SALE_NOTEBOOK);
}

void ab::SaleView::OnSaleNotebookClosing(wxAuiNotebookEvent& evt)
{
	auto idx = evt.GetSelection();
	if (idx == wxNOT_FOUND) return;
	auto& view = mSaleView[idx];
	auto model = dynamic_cast<ab::DataModel<grape::sale_display>*>(view->GetModel());
	if (!model) return;

	if (model->size() != 0 &&
		(wxMessageBox("Sale contains items that not are sold, are you sure you want to close sale?",
			"Sale", wxICON_WARNING | wxYES_NO) == wxNO))
	{
		evt.Veto();
		return;
	}

	
}

void ab::SaleView::OnSaleNotebookClosed(wxAuiNotebookEvent& evt)
{
	auto idx = evt.GetSelection();
	if (idx == wxNOT_FOUND) return;
	auto& view = mSaleView[idx];
	mSaleView.erase(std::next(mSaleView.begin(), idx));
	

	if (mSaleView.empty())
		mBook->SetSelection(SALE_EMPTY);
}

void ab::SaleView::OnSaleNotebookChanged(wxAuiNotebookEvent& evt)
{
	ClearTotals();
	UpdateTotals();
}

void ab::SaleView::OnOpenPacks(wxCommandEvent& evt)
{
}

void ab::SaleView::OnProductSearch(wxCommandEvent& evt)
{
	auto str = evt.GetString();
	if (str.empty()) {
		mSearchPopup->Dismiss();
		return;
	}

	wxPoint pos = mProductNameValue->ClientToScreen(wxPoint(0, 0));
	wxSize sz = mProductNameValue->GetClientSize();

	mSearchPopup->SetPosition(wxPoint{ pos.x, pos.y + sz.y + 5 });
	mSearchPopup->SetSize(FromDIP(wxSize(sz.x + 500, 400)));

	mSearchPopup->Search(str.ToStdString());
	mSearchPopup->Popup();
}

void ab::SaleView::OnProductSearchCleared(wxCommandEvent& evt)
{
	mSearchPopup->Dismiss();
	mProductNameValue->Clear();
}

void ab::SaleView::OnSearchedProduct(const grape::sale_display& saleproduct)
{
	mProductNameValue->Clear();
	if (mSaleNotebook->GetPageCount() == 0) {
		//add new page
		CreateView();
		mBook->SetSelection(SALE_NOTEBOOK);
	}

	int idx = mSaleNotebook->GetSelection();
	if (idx == wxNOT_FOUND) return;

	auto& view = mSaleView[idx];
	auto model = dynamic_cast<ab::DataModel<grape::sale_display>*>(view->GetModel());
	if (!model) return;
	auto str = boost::lexical_cast<std::string>(saleproduct.prod_id);
	if (std::any_of(model->begin(), model->end(), [&](auto& item) -> bool {
			auto& v = boost::fusion::at_c<2>(item);
			auto str2 = v[5].GetString().ToStdString();
			if (str == str2) {
				std::int64_t count = v[1].GetLong();
				v[1] = wxVariant(std::to_string(++count));
				return true;
			}
			else return false;
	})) {
		view->Refresh();
	}
	else {
		auto v = ab::make_variant<grape::sale_display>(std::move(const_cast<grape::sale_display&>(saleproduct)));
		model->Add(v);
	}
	UpdateTotals();
}

void ab::SaleView::ClearTotals()
{
	Freeze();
	mQuantityValue->SetLabel(std::to_string(0ll));
	mDiscountValue->SetLabel(fmt::format("{:cu}", pof::base::currency{}));
	mExtQuantityItem->SetLabel(std::to_string(0ll));
	mTotalQuantityValue->SetLabel(std::to_string(0ll)); //redundnat
	mTotalAmount->SetLabel(fmt::format("{:cu}", pof::base::currency{}));
	Thaw();
	mTextOutPut->Layout();
	mSalePaymentButtonsPane->Layout();
}

void ab::SaleView::UpdateTotals()
{
	int idx = mSaleNotebook->GetSelection();
	if (idx == wxNOT_FOUND) return;

	auto& view = mSaleView[idx];
	auto model = dynamic_cast<ab::DataModel<grape::sale_display>*>(view->GetModel());
	if (!model || model->empty()) return;

	std::uint64_t quan = 0;
	pof::base::currency total{};
	pof::base::currency totalDiscount{};
	auto funCur = [&](const std::string& string) -> pof::base::currency
		{
			auto pos = string.find_first_of(" ");
			auto str = string.substr(pos);
			auto i = std::ranges::remove_if(str,
				[&](char c) ->bool {return c == ','; });
			str.erase(i.begin(), i.end());

			return pof::base::currency(str);
		};

	for (auto& item : *model) {
		const auto& v = boost::fusion::at_c<2>(item);
		quan += v[1].GetLong();
		total += funCur(v[4].GetString().ToStdString());
		totalDiscount += funCur(v[3].GetString().ToStdString());
	}

	Freeze();
	mQuantityValue->SetLabel(std::to_string(model->size()));
	mDiscountValue->SetLabel(fmt::format("{:cu}", totalDiscount));
	mExtQuantityItem->SetLabel(std::to_string(quan));
	mTotalQuantityValue->SetLabel(std::to_string(quan)); //redundnat
	mTotalAmount->SetLabel(fmt::format("{:cu}", total));
	Thaw();
	mTextOutPut->Layout();
	mSalePaymentButtonsPane->Layout();
}

void ab::SearchPopup::SetNext(bool forward)
{
	auto item = GetSelected();
	if (!item.IsOk()) return;

	size_t i = mTableModel->GetRow(item);
	if (forward) i++;
	else         i--;

	const size_t size = mTableModel->size();
	i = std::clamp(i, 0ull, (size - 1));
	
	mTable->EnsureVisible(ab::DataModel<ab::pproduct>::ToDataViewItem(i));
	mTable->Select(ab::DataModel<ab::pproduct>::ToDataViewItem(i));
}

void ab::SearchPopup::SetActivated()
{
	auto item = GetSelected();
	if (!item.IsOk()) return;

	Dismiss();

	const int r = mTableModel->GetRow(item);
	auto row = ab::make_struct<ab::pproduct>(mTableModel->GetRow(r));
	if (!CheckProduct(row)) return;
	grape::sale_display sa;
	sa.prod_id    = row.id;
	sa.name       = row.name;
	sa.quantity   = 1;
	sa.unit_price = row.unit_price;
	sa.discount   = pof::base::currency{};
	sa.total      = row.unit_price;

	sSelectedSignal(sa);
}

void ab::SearchPopup::Search(const std::string& str)
{
	if (!mSearching) {
		if (std::ranges::all_of(str, [](char s) {return std::isspace(s); }))
			return;
		mBook->SetSelection(WAIT);
		mActivity->Start();
		mSearchString = str;
		boost::asio::post(wxGetApp().mTaskManager.tp(),
			std::bind_front(&ab::SearchPopup::SearchProducts, this, mSearchString));
	}
}

void ab::SearchPopup::SearchProducts(std::string&& sstring)
{
	mSearching = true;
	try {
		auto& app = wxGetApp();
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};
		boost::trim(sstring);
		boost::to_lower(sstring);
		boost::fusion::vector<std::uint32_t, std::string> searchT{ 0ul, std::forward<std::string>(sstring) };
	
		
		const size_t size = grape::serial::get_size(cred) + grape::serial::get_size(searchT);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		grape::serial::write(buf, searchT);
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());

		auto fut = sess->req(http::verb::get, "/product/search", std::move(body));
		
		auto resp = fut.get();
		if (resp.result() == http::status::not_found) {
			mBook->SetSelection(NO_RESULT);
			mSearching = false;
			return;
		}

		auto& b = resp.body();
		if (b.empty()) throw std::logic_error("no data receievd");
		auto&& [col, buf2] = grape::serial::read<grape::collection_type<ab::pproduct>>(boost::asio::buffer(b));
		auto& c = boost::fusion::at_c<0>(col);

		mTable->Freeze();
		mTableModel->Reload(c, 0, c.size(), c.size());

		mTable->Thaw();

		mActivity->Stop();
		mBook->SetSelection(DATA_VIEW);

		mSearching = false;

	}
	catch (const std::exception& exp) {
		spdlog::error(std::format("{} :{}", std::source_location::current(), exp.what()));
		mErrorText->SetLabel(exp.what());
		mBook->SetSelection(ERROR_PANE);
		mSearching = false;
	}
}

void ab::SearchPopup::SetupAuiTheme()
{
	auto auiart = mPopManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
	ab::AuiTheme::Register(std::bind_front(&ab::SearchPopup::OnAuiThemeChange, this));
}

void ab::SearchPopup::OnAuiThemeChange()
{
	auto auiart = mPopManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
}

void ab::SearchPopup::OnDataItemSelected(wxDataViewEvent& evt)
{
	auto item = evt.GetItem();
	if (!item.IsOk()) return;
	Dismiss();

	const int r = mTableModel->GetRow(item);
	auto row    = ab::make_struct<ab::pproduct>(mTableModel->GetRow(r));
	if (!CheckProduct(row)) return;
	grape::sale_display sa;
	sa.prod_id    = row.id;
	sa.name       = row.name;
	sa.quantity   = 1;
	sa.unit_price = row.unit_price;
	sa.discount   = pof::base::currency{};
	sa.total      = row.unit_price;

	sSelectedSignal(sa);
}

bool ab::SearchPopup::CheckProduct(const ab::pproduct& product)
{
	try {
		if (product.stock_count <= 0) {
			wxMessageBox(std::format("{} is out of stock, add stock to sell", product.name), "Sales", wxICON_WARNING | wxOK);
			return false;
		}

		if (product.cls == "POM" ||
			product.cls == "CONTROLLED") {
			wxMessageBox(std::format("{} requires a prescriprion to sell", product.name), "Sales", wxICON_WARNING | wxOK);
			return false;
		}
		auto& app = wxGetApp();
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};
		grape::uid_t p;
		boost::fusion::at_c<0>(p) = product.id;
		constexpr const size_t size = grape::serial::get_size(cred) +
			grape::serial::get_size(p);
		grape::body_type body(size, 0x00);
		auto buf1 = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf1, p);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
		->req(http::verb::get, "/product/expired/check", std::move(body));
		grape::session::response_type resp;
		{
			wxBusyInfo wait("Checking expired\nPlease wait...");
			resp = std::move(fut.get());
		}
		switch (resp.result())
		{
		case http::status::ok:
			wxMessageBox(std::format("{} is expired", product.name), "Sales", wxICON_WARNING | wxOK);
			return false;
		case http::status::not_found:
			break;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}
	}
	catch (const std::exception& exp) {
		wxMessageBox(exp.what(), "Sales", wxICON_ERROR | wxOK);
		return false;
	}
	return true;
}

ab::SearchPopup::SearchPopup(wxWindow* parent)
	: wxPopupTransientWindow(parent, wxBORDER_NONE), mPopManager(this, ab::AuiTheme::AUIMGRSTYLE), mSearching{false} {
	auto& app = wxGetApp();
	mBook = new wxSimplebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER | wxTAB_TRAVERSAL);
	mTableModel = new ab::DataModel<ab::pproduct>();
	mTable = new wxDataViewCtrl(mBook, 
		ID_DATA_VIEW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_ROW_LINES | wxDV_HORIZ_RULES);
	mTable->AssociateModel(mTableModel);
	mTableModel->DecRef();

	mTable->AppendTextColumn(wxT("Name"), 3, wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mTable->AppendTextColumn(wxT("Strength"), 1111, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mTable->AppendTextColumn(wxT("Formulation"), 6, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mTable->AppendTextColumn(wxT("Package Size"), 13, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	mTable->AppendTextColumn(wxT("Stock Count"), 14, wxDATAVIEW_CELL_INERT, FromDIP(100), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mTable->AppendTextColumn(wxT("Unit Price"), 11, wxDATAVIEW_CELL_INERT, FromDIP(70), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);

	mBook->AddPage(mTable, "View", false);
	
	std::tie(mNoResult, std::ignore, mNoResultRetry) = app.CreateEmptyPanel(mBook, "No such product in store", wxART_WARNING, wxSize(48, 48), wxART_MESSAGE_BOX);
	mNoResult->SetBackgroundColour(*wxWHITE);
	mBook->AddPage(mNoResult, "No result", false);
	
	std::tie(mWaitPanel, mActivity) = app.CreateWaitPanel(mBook, "Please wait..");
	mWaitPanel->SetBackgroundColour(*wxWHITE);
	mBook->AddPage(mWaitPanel, "Wait", true);
	


	std::tie(mErrorPanel, mErrorText, retry) = app.CreateEmptyPanel(mBook, "No connection", wxART_ERROR, wxSize(48, 48), wxART_MESSAGE_BOX);
	mBook->AddPage(mErrorPanel, "Error", false);
	retry->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		mBook->SetSelection(WAIT);
		boost::asio::post(wxGetApp().mTaskManager.tp(),
			std::bind_front(&ab::SearchPopup::SearchProducts, this, mSearchString));
	});
	SetupAuiTheme();
	mPopManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").Caption("Book").CenterPane().Show());
	mPopManager.Update();
}

void ab::SearchPopup::ChangeFont(const wxFont& font)
{
	mTable->SetFont(font);
}
