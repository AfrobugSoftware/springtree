#include "SearchPopup.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::SearchPopup, wxPopupTransientWindow)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::SearchPopup::ID_DATA_VIEW, ab::SearchPopup::OnDataItemSelected)
END_EVENT_TABLE()

static auto funCur = [](const std::string& string) -> pof::base::currency
{
		auto pos = string.find_first_of(" ");
		auto str = string.substr(pos);
		auto i = std::ranges::remove_if(str,
			[&](char c) ->bool {return c == ','; });
		str.erase(i.begin(), i.end());

		return pof::base::currency(str);
};

void ab::SearchPopup::SetNext(bool forward)
{
	auto item = GetSelected();
	if (!item.IsOk()) {
		mTable->Select(ab::DataModel<ab::pproduct>::ToDataViewItem(0));
		return;
	}

	size_t i = mTableModel->GetRow(item);
	if (forward) i++;
	else         i--;

	const size_t size = mTableModel->GetCount();
	i %= size;

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
	//send to modules that want raw data
	if(!sProductSignal.empty())
		sProductSignal(row); 
	else {
		if (!CheckProduct(row)) return;
		grape::sale_display sa;
		sa.prod_id = row.id;
		sa.name = row.name;
		sa.quantity = 1;
		sa.unit_price = row.unit_price;
		sa.unit_cost = row.cost_price;
		sa.discount = pof::base::currency{};
		sa.total = row.unit_price;

		sSelectedSignal(sa);
	}
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
	auto row = ab::make_struct<ab::pproduct>(mTableModel->GetRow(r));
	if(!sProductSignal.empty())
		sProductSignal(row);
	else {
		if (!CheckProduct(row)) return;
		grape::sale_display sa;
		sa.prod_id    = row.id;
		sa.name       = row.name;
		sa.quantity   = 1;
		sa.unit_price = row.unit_price;
		sa.unit_cost  = row.cost_price;
		sa.discount   = pof::base::currency{};
		sa.total      = row.unit_price;

		sSelectedSignal(sa);
	}
}

bool ab::SearchPopup::CheckProduct(const ab::pproduct& product)
{
	try {
		if (product.stock_count <= 0) {
			wxMessageBox(std::format("{} is out of stock, add stock to sell", product.name), "Sales", wxICON_WARNING | wxOK, GetParent());
			return false;
		}

		if (product.cls == "POM" ||
			product.cls == "CONTROLLED") {
			wxMessageBox(std::format("{} requires a prescriprion to sell", product.name), "Sales", wxICON_WARNING | wxOK, GetParent());
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
		wxMessageBox(exp.what(), "Sales", wxICON_ERROR | wxOK, GetParent());
		return false;
	}
	return true;
}

ab::SearchPopup::SearchPopup(wxWindow* parent)
	: wxPopupTransientWindow(parent, wxBORDER_NONE), mPopManager(this, ab::AuiTheme::AUIMGRSTYLE), mSearching{ false } {
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
	mErrorPanel->SetBackgroundColour(*wxWHITE);
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