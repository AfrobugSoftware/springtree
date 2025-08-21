#include "Reports.hpp"
#include "Application.hpp"
BEGIN_EVENT_TABLE(ab::Reports, wxPanel)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::Reports::ID_REPORT_LIST, ab::Reports::OnListActivated)
	EVT_TOOL(ab::Reports::ID_BACK, ab::Reports::OnBack)
	EVT_TOOL(ab::Reports::ID_DOWNLOAD_EXCEL, ab::Reports::OnDownloadExcel)
	EVT_DATE_CHANGED(ab::Reports::ID_START_DATE, ab::Reports::OnDateChanged)
	EVT_DATE_CHANGED(ab::Reports::ID_END_DATE, ab::Reports::OnDateChanged)
END_EVENT_TABLE()

ab::Reports::Reports(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	: wxPanel(parent, id, position, size, style), mManager{this, ab::AuiTheme::AUIMGRSTYLE} {
	mBook = new wxSimplebook(this, wxID_ANY);
	mStartDate = std::chrono::year_month_day{
		std::chrono::time_point_cast<std::chrono::sys_days::duration>(std::chrono::system_clock::now()) };
	mEndDate = std::chrono::year_month_day{
		std::chrono::time_point_cast<std::chrono::sys_days::duration>(std::chrono::system_clock::now()) };


	SetupAuiTheme();
	CreateToolbar();
	CreateListView();
	CreatePanels();
	CreateReportViews();

	mManager.AddPane(mBook, wxAuiPaneInfo().Name("Book").CaptionVisible(false).CenterPane().Show());
	mManager.Update();
}

void ab::Reports::SetupAuiTheme()
{
	auto auiart = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
	ab::AuiTheme::Register(std::bind_front(&ab::Reports::OnAuiThemeChange, this));
}

void ab::Reports::OnAuiThemeChange()
{
	auto auiart = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
}

void ab::Reports::ShowName(int page)
{
	mTools->Freeze();
	std::string name;
	switch (page)
	{
	case MAIN:
		name = "Reports";
		break;
	case EOD:
		name = "Reports - End of day";
		break;
	case EOM:
		name = "Reports - End of month";
		break;
	case RANGE:
		name = "Reports - Range";
		break;
	case PL:
		name = "Reports - Profit/Loss";
		break;
	case PURCHASE:
		name = "Reports - Purchase";
		break;
	default:
		name = "Invalid";
		break;
	}
	mReportName->SetLabel(name);
	mNameItem->SetMinSize(mReportName->GetSize());
	mTools->Realize();
	mManager.Update();

	mTools->Thaw();

}

void ab::Reports::UnLoad()
{
	for (auto& d : mReportViews)
	{
		if (!d) continue;
		const auto& s = d->GetStore();
		if(s->GetCount() != 0)
			s->DeleteAllItems();
	}
}

void ab::Reports::DoRetry()
{
	if (mLoadReports[page - 4]) {
		mWaitIndicator->Start();
		ShowName(page);
		mBook->SetSelection(WAIT);
		boost::asio::post(wxGetApp().mTaskManager.tp(), mLoadReports[page - 4]);
	}
}

void ab::Reports::CreateToolbar()
{
	mTools = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_HORZ_TEXT | wxAUI_TB_OVERFLOW | wxNO_BORDER);
	mTools->AddTool(ID_BACK, "Back", wxArtProvider::GetBitmap("back", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mTools->AddSpacer(FromDIP(5));
	mTools->AddSeparator();


	mReportName = new wxStaticText(mTools, wxID_ANY, "Reports", wxDefaultPosition, wxDefaultSize, 0);
	mReportName->SetFont(wxFontInfo().AntiAliased().Bold());
	mReportName->SetBackgroundColour(*wxWHITE);

	mTools->AddSpacer(FromDIP(5));
	mNameItem = mTools->AddControl(mReportName);



	mTools->AddStretchSpacer();

	mStartDatePicker = new wxDatePickerCtrl(mTools, ID_START_DATE, wxDateTime::Now(), wxDefaultPosition, FromDIP(wxSize(100, -1)), wxDP_DROPDOWN);
	mEndDatePicker = new wxDatePickerCtrl(mTools, ID_END_DATE, wxDateTime::Now(), wxDefaultPosition, FromDIP(wxSize(100, -1)), wxDP_DROPDOWN);

	mTools->AddControl(new wxStaticText(mTools, wxID_ANY, "Start:"), "Start date");
	mTools->AddSpacer(FromDIP(15));
	mTools->AddControl(mStartDatePicker);

	mTools->AddControl(new wxStaticText(mTools, wxID_ANY, "End:"), "End date");
	mTools->AddSpacer(FromDIP(15));
	mTools->AddControl(mEndDatePicker);

	mTools->Realize();
	mManager.AddPane(mTools, wxAuiPaneInfo().Name("Tools").ToolbarPane().Top().Row(1).MinSize(FromDIP(-1), FromDIP(30)).DockFixed().LeftDockable(false).RightDockable(false).Floatable(false).BottomDockable(false));
}

void ab::Reports::CreateListView()
{
	mReportList = new wxDataViewListCtrl(mBook, ID_REPORT_LIST, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES | wxDV_NO_HEADER);
	mReportList->AppendIconTextColumn("Selections");


	wxVector<wxVariant> data;
	data.push_back(wxVariant(wxDataViewIconText("End of day", wxArtProvider::GetBitmap("monitoring", wxART_OTHER, wxSize(16, 16)))));
	mReportList->AppendItem(data);

	data.clear();
	data.push_back(wxVariant(wxDataViewIconText("End of month", wxArtProvider::GetBitmap("insert_chart", wxART_OTHER, wxSize(16, 16)))));
	mReportList->AppendItem(data);

	data.clear();
	data.push_back(wxVariant(wxDataViewIconText("Range", wxArtProvider::GetBitmap("edit_note", wxART_OTHER, wxSize(16, 16)))));
	mReportList->AppendItem(data);

	data.clear();
	data.push_back(wxVariant(wxDataViewIconText("Profit/Loss", wxArtProvider::GetBitmap("payments", wxART_OTHER, wxSize(16, 16)))));
	mReportList->AppendItem(data);

	data.clear();
	data.push_back(wxVariant(wxDataViewIconText("Purchase", wxArtProvider::GetBitmap("bar_chart", wxART_OTHER, wxSize(16, 16)))));
	mReportList->AppendItem(data);

	mBook->AddPage(mReportList, "List", true);

}

void ab::Reports::CreatePanels()
{
	auto& app = wxGetApp();
	wxButton* addButton = nullptr;
	std::tie(mEmpty, std::ignore, addButton) = app.CreateEmptyPanel(mBook, "No report avaliable", "invoices");
	addButton->Hide();

	std::tie(mWaitPanel, mWaitIndicator) = app.CreateWaitPanel(mBook, "Please wait...");

	std::tie(mServerErrorPanel, mServerErrorText, addButton) = app.CreateEmptyPanel(mBook, "Server error", wxART_ERROR);
	addButton->SetLabel("Retry");
	addButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
		DoRetry();
		});
	addButton->SetBitmap(wxArtProvider::GetBitmap("retry", wxART_OTHER, FromDIP(wxSize(16, 16))));


	mBook->AddPage(mWaitPanel, "Wait", false);
	mBook->AddPage(mEmpty, "Empty", false);
	mBook->AddPage(mServerErrorPanel, "Error", false);
}

void ab::Reports::CreateReportViews()
{
	mReportViews = {0};
	mLoadReports = {0};


	//end of day
	mReportViews[0] = new wxDataViewListCtrl(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);
	mReportViews[0]->AppendDateColumn(wxT("Date"), 0, wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[0]->AppendTextColumn(wxT("Product"),   wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[0]->AppendTextColumn(wxT("Quantity"),   wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[0]->AppendTextColumn(wxT("Amount"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[0]->AppendTextColumn(wxT("Pay method"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mLoadReports[0] = std::bind_front(&ab::Reports::LoadEOD, this);
	mBook->AddPage(mReportViews[0], "EOD", false);

	mReportViews[1] = new wxDataViewListCtrl(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);
	mReportViews[1]->AppendDateColumn(wxT("Date"), 0, wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[1]->AppendTextColumn(wxT("Product"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[1]->AppendTextColumn(wxT("Quantity"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[1]->AppendTextColumn(wxT("Amount"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[1]->AppendTextColumn(wxT("Pay method"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mLoadReports[1] = std::bind_front(&ab::Reports::LoadEOM, this);
	mBook->AddPage(mReportViews[1], "EOM", false);

	mReportViews[2] = new wxDataViewListCtrl(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);
	mReportViews[2]->AppendDateColumn(wxT("Date"), 0, wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[2]->AppendTextColumn(wxT("Product"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[2]->AppendTextColumn(wxT("Quantity"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[2]->AppendTextColumn(wxT("Amount"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[2]->AppendTextColumn(wxT("Pay method"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mBook->AddPage(mReportViews[2], "RANGE", false);

	mReportViews[3] = new wxDataViewListCtrl(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);
	mReportViews[3]->AppendDateColumn(wxT("Date"), 0, wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[3]->AppendTextColumn(wxT("Product"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[3]->AppendTextColumn(wxT("Quantity"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[3]->AppendTextColumn(wxT("Amount Sell"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[3]->AppendTextColumn(wxT("Amount Cost"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[3]->AppendTextColumn(wxT("Profit/Loss"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mBook->AddPage(mReportViews[3], "PL", false);

	mReportViews[4] = new wxDataViewListCtrl(mBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);
	mReportViews[4]->AppendDateColumn(wxT("Date"), 0, wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[4]->AppendTextColumn(wxT("Product"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[4]->AppendTextColumn(wxT("Quantity"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mReportViews[4]->AppendTextColumn(wxT("Amount Cost"), wxDATAVIEW_CELL_INERT, FromDIP(150), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
	mBook->AddPage(mReportViews[4], "PL", false);

}

void ab::Reports::OnListActivated(wxDataViewEvent& evt)
{
	auto item = evt.GetItem();
	if (!item.IsOk()) return;
	int row = mReportList->GetSelectedRow(); //move past main

	if (mLoadReports[row]) {
		mWaitIndicator->Start();
		ShowName(row + 4);
		mBook->SetSelection(WAIT);
		boost::asio::post(wxGetApp().mTaskManager.tp(), mLoadReports[row]);
	}
}

void ab::Reports::OnDownloadExcel(wxCommandEvent& evt)
{
}

void ab::Reports::OnBack(wxCommandEvent& evt)
{
	switch (page)
	{
	case MAIN:
		mOnBack();
		mBook->SetSelection(MAIN);
		break;
	default:
		page = MAIN;
		mBook->SetSelection(MAIN);
		break;
	}
	ShowName(page);
}

void ab::Reports::OnDateChanged(wxDateEvent& evt)
{
	wxWindowID id = evt.GetId();
	const auto& date = evt.GetDate();
	const auto v = 
		std::chrono::time_point_cast<std::chrono::sys_days::duration>( std::chrono::system_clock::from_time_t(date.GetTicks()));
	switch (id)
	{
	case ID_START_DATE:
		mStartDate = std::chrono::year_month_day{ v };
		break;
	case ID_END_DATE:
		mEndDate = std::chrono::year_month_day{ v };
		break;
	}
}

void ab::Reports::LoadEOM()
{
	auto& app = wxGetApp();
	page = EOM;

	try {
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};
		grape::date_pack dp;
		dp.dt   = 1;
		dp.date = mStartDate.load();

		grape::page pg;
		pg.begin = 0;
		pg.limit = 10000;

		const size_t size = grape::serial::get_size(cred, dp, pg);
		grape::body_type body(size, 0x00);

		auto buf  = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf,  dp);
		auto buf3 = grape::serial::write(buf2, pg);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/reports/eom", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) throw std::logic_error("expected a body");

		auto&& [rep, rbuf] = grape::serial::read<
			grape::collection_type<grape::reports>>(boost::asio::buffer(rbody));
		auto& r = boost::fusion::at_c<0>(rep);
		auto& v = mReportViews[1];
		for (auto&& a : r) {
			wxVector<wxVariant> data;
			data.reserve(5);
			auto av = ab::make_variant<grape::reports>(std::forward<grape::reports>(a));
			
			data.push_back(av[2]);
			data.push_back(av[3]);
			data.push_back(av[4]);
			data.push_back(av[5]);
			data.push_back(av[7]);

			v->AppendItem(data);
		}

		mWaitIndicator->Stop();
		mBook->SetSelection(EOM);
	}
	catch (const std::exception& exp)
	{
		mServerErrorText->SetLabel(std::format("failed to load report\n{}", exp.what()));
		mBook->SetSelection(REPORT_ERR);
		mServerErrorPanel->Layout();
	}
}

void ab::Reports::LoadEOD()
{
	auto& app = wxGetApp();
	page = EOD;
	try {
		grape::credentials cred{
			app.mPharmacyManager.account.account_id,
			app.mPharmacyManager.account.session_id.value(),
			app.mPharmacyManager.pharmacy.id,
			app.mPharmacyManager.branch.id
		};
		grape::date_pack dp;
		dp.dt = 2;
		dp.date = mStartDate.load();

		grape::page pg;
		pg.begin = 0;
		pg.limit = 10000;

		const size_t size = grape::serial::get_size(cred, dp, pg);
		grape::body_type body(size, 0x00);

		auto buf = grape::serial::write(boost::asio::buffer(body), cred);
		auto buf2 = grape::serial::write(buf, dp);
		auto buf3 = grape::serial::write(buf2, pg);

		auto fut = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl())
			->req(http::verb::get, "/reports/eod", std::move(body));
		grape::session::response_type resp = fut.get();
		switch (resp.result())
		{
		case http::status::ok:
			break;
		case http::status::not_found:
			mBook->SetSelection(EMPTY);
			return;
		default:
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& rbody = resp.body();
		if (rbody.empty()) throw std::logic_error("expected a body");

		auto&& [rep, rbuf] = grape::serial::read<
			grape::collection_type<grape::reports>>(boost::asio::buffer(rbody));
		auto& r = boost::fusion::at_c<0>(rep);
		auto& v = mReportViews[0];
		for (auto&& a : r) {
			wxVector<wxVariant> data;
			data.reserve(5);
			auto av = ab::make_variant<grape::reports>(std::forward<grape::reports>(a));

			data.push_back(av[2]);
			data.push_back(av[3]);
			data.push_back(av[4]);
			data.push_back(av[5]);
			data.push_back(av[7]);

			v->AppendItem(data);
		}
		mWaitIndicator->Stop();
		mBook->SetSelection(EOD);
	}
	catch (const std::exception& exp)
	{
		mServerErrorText->SetLabel(std::format("failed to load report\n{}", exp.what()));
		mBook->SetSelection(REPORT_ERR);
		mServerErrorPanel->Layout();
	}
}
