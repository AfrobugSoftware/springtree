#include "Application.hpp"
#include "SetupWizard.hpp"

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

		mPharmacyManager.SearchPharmacies("zi");


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

		mMainFrame = new ab::MainFrame(nullptr, wxID_ANY, wxDefaultPosition, wxSize(400, 400));
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
	auto path = fs::current_path() / "settings.json";
	if (!fs::exists(path)) {
		//new installation, regesiter with grapejuice
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
		else if (boost::iequals(type, "application/octlet-stream")) {
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