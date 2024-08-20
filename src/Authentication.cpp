#include "Authentication.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::Register, wxDialog)
	EVT_CHECKBOX(ab::Register::ID_SHOW_PASSWORD, ab::Register::OnShowPassword)
	EVT_CLOSE(ab::Register::OnClose)
END_EVENT_TABLE()

ab::Register::Register(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
 : wxDialog(parent, id, title, pos, size, style) {
	auto& app = wxGetApp();
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
	SetBackgroundColour(*wxWHITE); //THEME
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	MainPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxVERTICAL);

	CreateAccount = new wxPanel(MainPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxVERTICAL);

	m_scrolledWindow1 = new wxScrolledWindow(CreateAccount, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
	m_scrolledWindow1->SetScrollRate(5, 5);
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);

	m_panel5 = new wxPanel(m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxVERTICAL);

	m_staticText12 = new wxStaticText(m_panel5, wxID_ANY, wxT("Creates an account with a pharmacy"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText12->Wrap(-1);
	bSizer5->Add(m_staticText12, 1, wxALL | wxEXPAND, 10);


	m_panel5->SetSizer(bSizer5);
	m_panel5->Layout();
	bSizer5->Fit(m_panel5);
	bSizer4->Add(m_panel5, 0, wxALL | wxEXPAND, 2);

	fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer1->AddGrowableCol(1);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	mLastNameLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Last Name:"), wxDefaultPosition, wxDefaultSize, 0);
	mLastNameLabel->Wrap(-1);
	fgSizer1->Add(mLastNameLabel, 0, wxALL, 5);

	mLastNameValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mLastNameValue->SetMaxLength(30);
	mLastNameValue->SetMinSize(wxSize(100, -1));
	mLastNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY | wxFILTER_ALPHA });

	fgSizer1->Add(mLastNameValue, 0, wxALL | wxEXPAND, 5);

	mFirstNameLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("First Name:"), wxDefaultPosition, wxDefaultSize, 0);
	mFirstNameLabel->Wrap(-1);
	fgSizer1->Add(mFirstNameLabel, 0, wxALL, 5);

	mFirstNameValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mFirstNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY | wxFILTER_ALPHA });
	fgSizer1->Add(mFirstNameValue, 0, wxALL | wxEXPAND, 5);

	mUserNameLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Username:"), wxDefaultPosition, wxDefaultSize, 0);
	mUserNameLabel->Wrap(-1);
	fgSizer1->Add(mUserNameLabel, 0, wxALL, 5);

	mUserNameValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mUserNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY | wxFILTER_ALPHA });
	fgSizer1->Add(mUserNameValue, 0, wxALL | wxEXPAND, 5);

	mAccountTypeLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Account Type"), wxDefaultPosition, wxDefaultSize, 0);
	mAccountTypeLabel->Wrap(-1);
	fgSizer1->Add(mAccountTypeLabel, 0, wxALL, 5);

	//arranged according to the account priv bits
	wxString mAccountTypeChoices[] = { wxT("SUPERINTENDENT PHARMACIST"),
			wxT("PHARMACY TECH"),
			wxT("DISPENSER"),
			wxT("SALES ASSISTANT"),
			wxT("INTERN PHARMACIST"),
			wxT("STUDENT PHARMACIST"),
			wxT("MANAGER"),
	};
	int mAccountTypeNChoices = sizeof(mAccountTypeChoices) / sizeof(wxString);
	mAccountType = new wxChoice(m_scrolledWindow1, ID_ACCOUNT_TYPE, wxDefaultPosition, wxDefaultSize, mAccountTypeNChoices, mAccountTypeChoices, 0);
	fgSizer1->Add(mAccountType, 0, wxALL | wxEXPAND, 5);

	mEmailLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Email"), wxDefaultPosition, wxDefaultSize, 0);
	mEmailLabel->Wrap(-1);
	fgSizer1->Add(mEmailLabel, 0, wxALL, 5);

	mEmailValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mEmailValue->SetValidator(ab::RegexValidator(std::regex(R"(^[\w-\.]+@([\w-]+\.)+[\w-]{2,4}$)"), "Invalid email address"));
	fgSizer1->Add(mEmailValue, 0, wxALL | wxEXPAND, 5);

	mPhoneNo = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Phone No"), wxDefaultPosition, wxDefaultSize, 0);
	mPhoneNo->Wrap(-1);
	fgSizer1->Add(mPhoneNo, 0, wxALL, 5);

	mPhoneNoValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mPhoneNoValue->SetValidator(ab::RegexValidator(std::regex("(0|91)?[6-9][0-9]{9}"), "Invalid phone number"));
	mPhoneNoValue->SetMaxLength(11);
	fgSizer1->Add(mPhoneNoValue, 0, wxALL | wxEXPAND, 5);



	mPasswordLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Password"), wxDefaultPosition, wxDefaultSize, 0);
	mPasswordLabel->Wrap(-1);
	fgSizer1->Add(mPasswordLabel, 0, wxALL, 5);

	mPasswordValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	mPasswordValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
	fgSizer1->Add(mPasswordValue, 0, wxALL | wxEXPAND, 5);

	mConfirmPasswordLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Confirm Password"), wxDefaultPosition, wxDefaultSize, 0);
	mConfirmPasswordLabel->Wrap(-1);
	fgSizer1->Add(mConfirmPasswordLabel, 0, wxALL, 5);

	mConfirmPasswordValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	mConfirmPasswordValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
	fgSizer1->Add(mConfirmPasswordValue, 0, wxALL | wxEXPAND, 5);


	fgSizer1->Add(0, 0, 1, wxEXPAND, 5);

	m_checkBox1 = new wxCheckBox(m_scrolledWindow1, ID_SHOW_PASSWORD, wxT("Show password"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
	fgSizer1->Add(m_checkBox1, 0, wxALL, 5);

	mRegNumberLabel = new wxStaticText(m_scrolledWindow1, wxID_ANY, wxT("Reg-No"), wxDefaultPosition, wxDefaultSize, 0);
	mRegNumberLabel->Wrap(-1);
	fgSizer1->Add(mRegNumberLabel, 0, wxALL, 5);

	mRegNumValue = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(mRegNumValue, 0, wxALL | wxEXPAND, 5);


	fgSizer1->Add(0, 0, 1, wxEXPAND, 5);

	wxString m_radioBox2Choices[] = { wxT("Principal Pharmacist"), wxT("Loccum Pharmacist") };
	int m_radioBox2NChoices = sizeof(m_radioBox2Choices) / sizeof(wxString);
	m_radioBox2 = new wxRadioBox(m_scrolledWindow1, wxID_ANY, wxT("Phamacist role"), wxDefaultPosition, wxDefaultSize, m_radioBox2NChoices, m_radioBox2Choices, 1, wxRA_SPECIFY_COLS);
	fgSizer1->Add(m_radioBox2, 0, wxALL | wxEXPAND, 5);


	auto m_panel4 = new wxPanel(m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer(new wxStaticBox(m_panel4, wxID_ANY, wxT("Security questions")), wxVERTICAL);

	mSecurityQuestions = new wxChoice(sbSizer7->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, app.mSecurityQuestions, 0);
	sbSizer7->Add(mSecurityQuestions, 0, wxALL | wxEXPAND, 5);

	mSecurityAnswer = new wxTextCtrl(sbSizer7->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mSecurityAnswer->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
	sbSizer7->Add(mSecurityAnswer, 0, wxALL | wxEXPAND, 5);

	m_panel4->SetSizer(sbSizer7);
	m_panel4->Layout();
	sbSizer7->Fit(m_panel4);

	fgSizer1->Add(0, 0, 1, wxEXPAND, 5);
	fgSizer1->Add(m_panel4, 0, wxALL | wxEXPAND, 5);

	bSizer4->Add(fgSizer1, 1, wxALL | wxEXPAND, 10);


	m_scrolledWindow1->SetSizer(bSizer4);
	m_scrolledWindow1->Layout();
	bSizer4->Fit(m_scrolledWindow1);
	bSizer8->Add(m_scrolledWindow1, 1, wxEXPAND | wxALL, 5);


	CreateAccount->SetSizer(bSizer8);
	CreateAccount->Layout();
	//bSizer8->Fit( CreateAccount );
	bSizer3->Add(CreateAccount, 1, wxEXPAND | wxALL, 5);

	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3Save = new wxButton(MainPane, wxID_OK);
	m_sdbSizer3->AddButton(m_sdbSizer3Save);
	m_sdbSizer3Cancel = new wxButton(MainPane, wxID_CANCEL);
	m_sdbSizer3->AddButton(m_sdbSizer3Cancel);
	m_sdbSizer3->Realize();

	bSizer3->Add(m_sdbSizer3, 0, wxALL | wxEXPAND, 10);


	MainPane->SetSizer(bSizer3);
	MainPane->Layout();
	bSizer3->Fit(MainPane);
	bSizer2->Add(MainPane, 1, wxEXPAND | wxALL, 0);


	this->SetSizer(bSizer2);
	this->Layout();

	this->Centre(wxBOTH);

	SetIcon(app.mAppIcon);
}

ab::Register::~Register()
{
}

void ab::Register::OnClose(wxCloseEvent& evt)
{
	EndModal(wxID_CANCEL);
}

void ab::Register::OnShowPassword(wxCommandEvent& evt)
{
	auto pflags = mPasswordValue->GetWindowStyleFlag();
	auto cflags = mConfirmPasswordValue->GetWindowStyleFlag();

	if (evt.IsChecked()) {
		pflags = (pflags & (~wxTE_PASSWORD));
		cflags = (cflags & (~wxTE_PASSWORD));

	}
	else {
		pflags = pflags | wxTE_PASSWORD;
		cflags = cflags | wxTE_PASSWORD;
	}


	Freeze();
	auto pv = mPasswordValue->GetValue();
	auto cv = mConfirmPasswordValue->GetValue();

	delete mPasswordValue;
	delete mConfirmPasswordValue;

	auto newPass = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, pv, wxDefaultPosition, wxDefaultSize, pflags);
	auto newConfirm = new wxTextCtrl(m_scrolledWindow1, wxID_ANY, cv, wxDefaultPosition, wxDefaultSize, cflags);

	fgSizer1->Insert(13, newPass, wxSizerFlags().Expand().Border(wxALL, 5));
	fgSizer1->Insert(15, newConfirm, wxSizerFlags().Expand().Border(wxALL, 5));


	mPasswordValue = newPass;
	mConfirmPasswordValue = newConfirm;

	m_radioBox2->Layout();
	fgSizer1->Layout();
	Thaw();
	m_scrolledWindow1->Refresh();
}

bool ab::Register::TransferDataFromWindow()
{
	try {
		wxBusyInfo wait("Registering user on grape juice\nPlease wait...");
		auto& app = wxGetApp();
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		std::string name = mUserNameValue->GetValue().ToStdString();
		boost::trim(name);
		boost::to_lower(name);

		std::string target = std::format("/account/checkname/{}", name);
		auto fut = sess->req(http::verb::get, target, { app.mPharmacyManager.pharmacy.id.begin(), app.mPharmacyManager.pharmacy.id.end() });
		auto resp = fut.get();
		if (resp.result() != http::status::not_found) {
			wxMessageBox("Username already exists on this pharmacy, please try another name", "Registration", wxICON_WARNING | wxOK);
			return false;
		}

		if (!boost::equals(mPasswordValue->GetValue(),mConfirmPasswordValue->GetValue())) {
			wxMessageBox("Password and Confirm password mismatch", "Registration", wxICON_WARNING | wxOK);
			return false;
		}
		
		int sel = mAccountType->GetSelection();
		if (sel == wxNOT_FOUND) {
			wxMessageBox("Please select an account type for your account", "Registration", wxICON_WARNING | wxOK);
			return false;
		}

		if (sel == 0 && mRegNumValue->GetValue().IsEmpty()) {
			wxMessageBox("Reg number cannot be empty for a superintendent pharmacist", "Registration", wxICON_WARNING | wxOK);
			return false;
		}
		const auto pass = mPasswordValue->GetValue().ToStdString();
		const auto hash = bcrypt::generateHash(pass);

		grape::account account;
		account.account_id = boost::uuids::nil_uuid();
		account.first_name = mFirstNameValue->GetValue().ToStdString();
		account.last_name = mLastNameValue->GetValue().ToStdString();
		account.passhash  = hash;
		account.username = name;
		account.email = mEmailValue->GetValue().ToStdString();
		account.phonenumber = mPhoneNoValue->GetValue().ToStdString();
		account.regnumber = mRegNumValue->GetValue().ToStdString();
		account.sec_que = mSecurityQuestions->GetStringSelection();
		account.sec_ans = mSecurityAnswer->GetValue();

		const size_t size = grape::serial::get_size(account);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		grape::serial::write(boost::asio::buffer(body), account);

		fut = sess->req(http::verb::post, "/account/signup", std::move(body));
		resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}
		return true;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(exp.what(), "Registration",wxICON_ERROR | wxOK);
	}
	return false;
}

BEGIN_EVENT_TABLE(ab::SignIn, wxDialog)
	EVT_BUTTON(ab::SignIn::ID_SIGNUP, ab::SignIn::OnSignUp)
	EVT_CLOSE(ab::SignIn::OnClose)
END_EVENT_TABLE()

ab::SignIn::SignIn(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
 : wxDialog(parent, id, wxT("PharmaOffice - Enterprise"), pos, size, style) {
	this->SetSize(FromDIP(size));
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	RootPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	RootPane->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	m_panel2 = new wxPanel(RootPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);


	bSizer3->Add(0, 0, 1, wxEXPAND, FromDIP(5));


	m_panel2->SetSizer(bSizer3);
	m_panel2->Layout();
	bSizer3->Fit(m_panel2);
	bSizer2->Add(m_panel2, 1, wxEXPAND | wxALL, FromDIP(5));

	m_panel3 = new wxPanel(RootPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxVERTICAL);


	bSizer5->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	m_bitmap1 = new wxStaticBitmap(m_panel3, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(m_bitmap1, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

	mWelcomText = new wxStaticText(m_panel3, wxID_ANY, wxT("WELCOME TO PHARMAOFFICE"), wxDefaultPosition, wxDefaultSize, 0);
	mWelcomText->Wrap(-1);
	mWelcomText->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString));

	bSizer5->Add(mWelcomText, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));


	bSizer5->Add(0, 30, 0, wxEXPAND, FromDIP(5));

	mUserName = new wxTextCtrl(m_panel3, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mUserName->SetMinSize(FromDIP(wxSize(300, -1)));
	mUserName->SetHint("Username");
	//mUserName->SetValidator(wxTextValidator{ wxFILTER_EMPTY });

	bSizer5->Add(mUserName, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

	mPassword = new wxTextCtrl(m_panel3, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	mPassword->SetMinSize(FromDIP(wxSize(300, -1)));
	mPassword->SetHint("Password");
	//mPassword->SetValidator(wxTextValidator{ wxFILTER_EMPTY });

	bSizer5->Add(mPassword, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

	mKeepMeSigned = new wxCheckBox(m_panel3, wxID_ANY, wxT("Keep me signed in"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(mKeepMeSigned, 0, wxALL, FromDIP(5));

	bSizer5->Add(0, 20, 0, wxEXPAND, 5);

	m_panel5 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxHORIZONTAL);


	bSizer6->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	mLogOn = new wxButton(m_panel5, wxID_OK, wxT("Log on"), wxDefaultPosition, wxDefaultSize, 0);
	mLogOn->SetDefault();
	bSizer6->Add(mLogOn, 0, wxALL, FromDIP(5));

	mSignup = new wxButton(m_panel5, ID_SIGNUP, wxT("Sign Up"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(mSignup, 0, wxALL, FromDIP(5));


	m_panel5->SetSizer(bSizer6);
	m_panel5->Layout();
	bSizer6->Fit(m_panel5);
	bSizer5->Add(m_panel5, 0, wxEXPAND | wxALL, 0);


	bSizer5->Add(FromDIP(0), FromDIP(20), 0, wxEXPAND, FromDIP(5));

	mPharmacySignupPanel = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer(wxHORIZONTAL);


	bSizer7->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	mForgotPasswordLink = new wxHyperlinkCtrl(mPharmacySignupPanel, ID_FORGOT_PASS, wxT("Forgot password"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
	bSizer7->Add(mForgotPasswordLink, 0, wxALL, FromDIP(5));

	mHelpLink = new wxHyperlinkCtrl(mPharmacySignupPanel, ID_HELP, wxT("Help"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
	bSizer7->Add(mHelpLink, 0, wxALL, FromDIP(5));


	mPharmacySignupPanel->SetSizer(bSizer7);
	mPharmacySignupPanel->Layout();
	bSizer7->Fit(mPharmacySignupPanel);
	bSizer5->Add(mPharmacySignupPanel, 1, wxEXPAND | wxALL, FromDIP(5));



	m_panel3->SetSizer(bSizer5);
	m_panel3->Layout();
	bSizer5->Fit(m_panel3);
	bSizer2->Add(m_panel3, 1, wxEXPAND | wxALL, FromDIP(5));

	m_panel4 = new wxPanel(RootPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);


	bSizer4->Add(0, 0, 1, wxEXPAND, FromDIP(5));


	m_panel4->SetSizer(bSizer4);
	m_panel4->Layout();
	bSizer4->Fit(m_panel4);
	bSizer2->Add(m_panel4, 1, wxEXPAND | wxALL, FromDIP(5));


	RootPane->SetSizer(bSizer2);
	RootPane->Layout();
	bSizer2->Fit(RootPane);
	bSizer1->Add(RootPane, 1, wxEXPAND | wxALL, 0);


	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);

	SetIcon(wxGetApp().mAppIcon);
}

ab::SignIn::~SignIn()
{
}

void ab::SignIn::OnClose(wxCloseEvent& evt)
{
	EndModal(wxID_CANCEL);
}

void ab::SignIn::OnSignUp(wxCommandEvent& evt)
{
	ab::Register reg(nullptr);
	if (reg.ShowModal() == wxID_OK) {
		wxMessageBox("Successfully created account", "Registration", wxICON_INFORMATION | wxOK);
	}
}


void ab::SignIn::OnForgotPassword(wxHyperlinkEvent& evt)
{
}

bool ab::SignIn::TransferDataFromWindow()
{
	auto& app = wxGetApp();
	try {
		
		wxBusyInfo wait("Signing in\nPlease wait...");
		
		grape::account_cred cred;
		cred.pharmacy_id = app.mPharmacyManager.pharmacy.id;
		cred.username = mUserName->GetValue();
		cred.password = mPassword->GetValue();
		const size_t size = grape::serial::get_size(cred);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		grape::serial::write(boost::asio::buffer(body), cred);


		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		auto fut = sess->req(http::verb::post, "/account/signin", std::move(body));

		auto resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& b = resp.body();
		if (b.empty()) throw std::invalid_argument("No body in response from grape juice");

		auto&& [s, buff] = grape::serial::read<grape::account>(boost::asio::buffer(b));
		app.mPharmacyManager.account = std::move(s);
		
		//store the session id and time some where
		auto sess_settings = js::json::object();
		sess_settings["remember_me"] = mKeepMeSigned->GetValue();
		sess_settings["id"] = boost::lexical_cast<std::string>(app.mPharmacyManager.account.session_id.value());
		sess_settings["session_start_time"] = static_cast<std::uint64_t>(app.mPharmacyManager.account.session_start_time.value().time_since_epoch().count());

		app.settings["session"] = sess_settings;
		return true;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(exp.what(), "Sign up", wxICON_ERROR | wxOK);
	}
	return false;
}