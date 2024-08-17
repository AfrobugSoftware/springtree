#include "Application.hpp"
#include "SetupWizard.hpp"

#include "DataModel.hpp"

IMPLEMENT_APP(ab::Application)
boost::asio::ip::tcp::endpoint m_globalendpoint;

ab::Application::Application() {
	mAsserts = fs::current_path() / "asserts";

	//for test, should be got from the settings file
	m_globalendpoint = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string("127.0.0.1"),
		8080
	);
}

bool ab::Application::OnInit()
{
	try {
		if (!wxApp::OnInit()) return false;

		wxInitAllImageHandlers();
		wxArtProvider::Push(new ab::ArtProvider);

		wxDialog::EnableLayoutAdaptation(true);
		mAppIcon.CopyFromBitmap(wxArtProvider::GetBitmap("pharmaofficeico"));


		CreateColourDatabase();
		CreateSecurityQuestions();
		auto logpath = fs::current_path() / ".logs" / "log.txt";
		auto my_logger = spdlog::basic_logger_mt("springtree", logpath.string(), true);
		spdlog::set_default_logger(my_logger);


		//mPharmacyManager.GetPharmacies();
		//ab::DataModel<grape::product> mProductModel;

		if (!LoadSettings()) {
			ab::SetupWizard* wizard = new ab::SetupWizard(nullptr);
			wizard->RunWizard(wizard->GetFirstPage());

			bool state = wizard->GetState();
			wizard->Destroy();
			delete wizard;
			

			if (!state) {
				OnExit();
				return false;
			}

		}

		mMainFrame = new ab::MainFrame(nullptr, wxID_ANY, wxDefaultPosition, wxSize(822, 762));
		mMainFrame->Show();
		return true;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());

		wxMessageBox(exp.what(), "FATAL ERROR, Call D-GLOPA admin", wxICON_ERROR | wxOK);
		OnExit();
		return false;
	}
}

int ab::Application::OnExit()
{
	mNetManager.stop();
	return 0;
}

void ab::Application::DecorateSplashScreen(wxBitmap& bmp)
{
	// use a memory DC to draw directly onto the bitmap
	wxMemoryDC memDc(bmp);

	// draw an orange box (with black outline) at the bottom of the splashscreen.
	// this box will be 10% of the height of the bitmap, and be at the bottom.
	const wxRect bannerRect(wxPoint(0, (bmp.GetHeight() / 10) * 9),
		wxPoint(bmp.GetWidth(), bmp.GetHeight()));
	wxDCBrushChanger bc(memDc, wxBrush(wxColour(255, 102, 0)));
	memDc.DrawRectangle(bannerRect);
	memDc.DrawLine(bannerRect.GetTopLeft(), bannerRect.GetTopRight());

	// dynamically get the wxWidgets version to display
	wxString description = wxString::Format("PharmaOffice %s", gVersion);
	// create a copyright notice that uses the year that this file was compiled
	wxString year(__DATE__);
	wxString copyrightLabel = wxString::Format("%s%s D-glopa Nigeria limited. %s",
		wxString::FromUTF8("\xc2\xa9"), year.Mid(year.length() - 4),
		"All rights reserved.");

	// draw the (white) labels inside of our orange box (at the bottom of the splashscreen)
	memDc.SetTextForeground(*wxWHITE);
	// draw the "wxWidget" label on the left side, vertically centered.
	// note that we deflate the banner rect a little bit horizontally
	// so that the text has some padding to its left.
	memDc.DrawLabel(description, bannerRect.Deflate(5, 0), wxALIGN_CENTRE_VERTICAL | wxALIGN_LEFT);

	// draw the copyright label on the right side
	memDc.SetFont(wxFontInfo(8));
	memDc.DrawLabel(copyrightLabel, bannerRect.Deflate(5, 0), wxALIGN_CENTRE_VERTICAL | wxALIGN_RIGHT);
}

bool ab::Application::LoadSettings()
{
	auto path = fs::current_path() / ".data"s / "settings.json"s;
	if (!fs::exists(path)) return false;
	try {
		std::fstream file(path, std::ios::in);
		std::ostringstream os;
		os << file.rdbuf();

		//put a Busy wait here
		
		js::json settings = js::json::parse(os.str());
		boost::uuids::uuid id = boost::lexical_cast<boost::uuids::uuid>(static_cast<std::string>(settings["pharmacy_id"]));

		//update to getting pharmacy
		//get pharmacy from grape juice
		auto sess = std::make_shared<grape::session>(mNetManager.io(),mNetManager.ssl());
		grape::session::request_type::body_type::value_type value(id.begin(), id.end());
		auto fut = sess->req(http::verb::get, "/pharmacy/getbyid", std::move(value));
		auto resp = fut.get();

		if (resp.result() != http::status::ok) {
			throw std::logic_error(ParseServerError(resp));
		}

		auto&& [p, buf] = grape::serial::read<grape::pharmacy>(boost::asio::buffer(resp.body()));
		mPharmacyManager.pharmacy = std::forward<grape::pharmacy>(p);

		//update to getting branch details
		id = boost::lexical_cast<boost::uuids::uuid>(static_cast<std::string>(settings["branch_id"]));
		grape::session::request_type::body_type::value_type value2(id.begin(), id.end());
		fut = sess->req(http::verb::get, "/pharmacy/branch/getbyid", std::move(value2));
		resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(ParseServerError(resp));
		}

		auto&& [br, buf2] = grape::serial::read<grape::branch>(boost::asio::buffer(resp.body()));
		mPharmacyManager.branch = std::forward<grape::branch>(br);

		//update to gettings address details
		id = mPharmacyManager.branch.address_id;
		grape::session::request_type::body_type::value_type value3(id.begin(), id.end());
		fut = sess->req(http::verb::get, "/pharmacy/address", std::move(value3));
		resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(ParseServerError(resp));
		}

		auto&& [ady, buf3] = grape::serial::read<grape::address>(boost::asio::buffer(resp.body()));
		mPharmacyManager.address = std::forward<grape::address>(ady);
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		return false;
	}
	return true;
}

bool ab::Application::SaveSettings()
{
	return false;
}

void ab::Application::CreateAddress()
{
	mAppAddress.id = boost::uuids::random_generator_mt19937{}();
	mAppAddress.country = "nigeria";
	mAppAddress.num = "10";
	mAppAddress.lga = "isoko north";
	mAppAddress.street = "dbs road";
	mAppAddress.state = "delta";
}

void ab::Application::CreateColourDatabase()
{
	//wxTheColourDatabase->AddColour("module", wxColour(0xFF, 0xCA, 0x28, 50));
	wxTheColourDatabase->AddColour("module", wxColour(0xFF, 0xFF, 0xFF, 50));
	wxTheColourDatabase->AddColour("Aqua", wxColour(240, 255, 255));
	wxTheColourDatabase->AddColour("Navajo_white", wxColour(255, 222, 173));
	wxTheColourDatabase->AddColour("Tomato", wxColour(255, 99, 71));
	wxTheColourDatabase->AddColour("Papaya whip", wxColour(255, 239, 213));
}

void ab::Application::CreateSecurityQuestions()
{
	mSecurityQuestions.push_back("What was the name of your first school teacher?");
	mSecurityQuestions.push_back("What year did you enter college?");
	mSecurityQuestions.push_back("What is your grandmother’s maiden name?");
	mSecurityQuestions.push_back("What color do you like the most?");
	mSecurityQuestions.push_back("What’s your favorite artist?");
	mSecurityQuestions.push_back("What book do you recommend to your friends?");
}

bool ab::Application::SendPing()
{
	try {
		auto sess = std::make_shared<grape::session>(mNetManager.io(),
			mNetManager.ssl());
		const ssize_t size = grape::serial::get_size(mAppDetails) +
			grape::serial::get_size(mPharmacyManager.address);
		grape::session::request_type::body_type::value_type body(size, 0x00);

		auto buf = grape::serial::write(boost::asio::buffer(body), mAppDetails);
		grape::serial::write(buf, mPharmacyManager.address);

		auto fut = sess->req(http::verb::post, "/app/ping", std::move(body));
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
	}
	return false;
}

std::tuple<wxPanel*, wxStaticText*, wxButton*> ab::Application::CreateEmptyPanel(wxWindow* parent, const std::string& text, const std::string& img, const std::string& client)
{
	wxPanel * mEmpty = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	wxPanel* m5 = new wxPanel(mEmpty, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxHORIZONTAL);


	bSizer8->Add(0, 0, 1, wxEXPAND, 5);

	wxPanel* m7 = new wxPanel(m5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer(wxVERTICAL);


	bSizer9->Add(0, 0, 1, wxEXPAND, 5);

	wxStaticBitmap* b1 = new wxStaticBitmap(m7, wxID_ANY, wxArtProvider::GetBitmap(img, client), wxDefaultPosition, wxDefaultSize, 0);
	bSizer9->Add(b1, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxStaticText* mEmptyStr = new wxStaticText(m7, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, 0);
	mEmptyStr->Wrap(-1);
	bSizer9->Add(mEmptyStr, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	wxButton* btn = new wxButton(m7, wxID_ANY);
	btn->SetBitmap(wxArtProvider::GetBitmap("add_task", wxART_OTHER));
	btn->SetLabel("button");
	btn->SetBackgroundColour(*wxWHITE);
	bSizer9->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);


	bSizer9->Add(0, 0, 1, wxEXPAND, 5);


	m7->SetSizer(bSizer9);
	m7->Layout();
	bSizer9->Fit(m7);
	bSizer8->Add(m7, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer8->Add(0, 0, 1, wxEXPAND, 5);


	m5->SetSizer(bSizer8);
	m5->Layout();
	bSizer8->Fit(m5);
	bSizer6->Add(m5, 1, wxEXPAND | wxALL, 5);


	mEmpty->SetSizer(bSizer6);
	mEmpty->Layout();

	return std::make_tuple(mEmpty, mEmptyStr, btn);
}

std::pair<wxPanel*, wxActivityIndicator*> ab::Application::CreateWaitPanel(wxWindow* parent, const std::string& text)
{
	wxPanel* mEmpty = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	wxPanel* m5 = new wxPanel(mEmpty, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxHORIZONTAL);


	bSizer8->Add(0, 0, 1, wxEXPAND, 5);

	wxPanel* m7 = new wxPanel(m5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer(wxVERTICAL);


	bSizer9->Add(0, 0, 1, wxEXPAND, 5);
	wxActivityIndicator* b1 = new wxActivityIndicator(m7, wxID_ANY);
	bSizer9->Add(b1, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxStaticText* mEmptyStr = new wxStaticText(m7, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, 0);
	mEmptyStr->Wrap(-1);
	bSizer9->Add(mEmptyStr, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	bSizer9->Add(0, 0, 1, wxEXPAND, 5);


	m7->SetSizer(bSizer9);
	m7->Layout();
	bSizer9->Fit(m7);
	bSizer8->Add(m7, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer8->Add(0, 0, 1, wxEXPAND, 5);


	m5->SetSizer(bSizer8);
	m5->Layout();
	bSizer8->Fit(m5);
	bSizer6->Add(m5, 1, wxEXPAND | wxALL, 5);


	mEmpty->SetSizer(bSizer6);
	mEmpty->Layout();

	return std::make_pair(mEmpty, b1);
}

std::string ab::Application::ParseServerError(const grape::session::response_type& resp)
{
	try {
		if (!resp.has_content_length()) return ""s;
		auto type = resp.at(http::field::content_type);
		if (boost::iequals(type, "application/json")) {
			//parse as json
			auto& body = resp.body();
			std::string jText(body.size(), 0x00);
			std::ranges::copy(body, jText.begin());

			js::json obj = js::json::parse(jText);
			
			return obj["result_message"];
		}
		else if (boost::iequals(type, "application/octet-stream")) {
			//parse as a stream of byte
			auto&& [message, buf] = grape::serial::read<grape::result>(boost::asio::buffer(resp.body()));
			return message.message;
		}

		return ""s;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		return ""s;
	}
}

bool ab::Application::LoadAppDetails()
{
	return false;
}

ab::RegexValidator::RegexValidator(std::regex&& reg, const std::string& errorstr)
	: pattern(reg), estr(errorstr) {

}

wxObject* ab::RegexValidator::Clone() const
{
	auto p = pattern;
	auto s = estr;
	auto sp = new RegexValidator(std::move(p), s);
	return sp;
}

bool ab::RegexValidator::TransferFromWindow()
{
	return true;
}

bool ab::RegexValidator::TransferToWindow()
{
	return true;
}

bool ab::RegexValidator::Validate(wxWindow* parent)
{
	wxTextCtrl* control = dynamic_cast<wxTextCtrl*>(parent);
	if (!control) return false;


	auto&& v = control->GetValue().ToStdString();
	if (!v.empty() && !std::regex_match(v, pattern)) {
		wxMessageBox(estr, "Validator", wxICON_WARNING | wxOK);
		return false;
	}
	return true;
}

wxString ab::RegexValidator::IsValid(const wxString& val) const
{
	if (!std::regex_match(val.ToStdString(), pattern)) {
		return estr;
	}
	return wxEmptyString;
}