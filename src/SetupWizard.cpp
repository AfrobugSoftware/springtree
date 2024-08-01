#include "SetupWizard.hpp"


BEGIN_EVENT_TABLE(ab::SetupWizard, wxWizard)
	EVT_WIZARD_FINISHED(wxID_ANY, ab::SetupWizard::OnFinished)
	EVT_BUTTON(ab::SetupWizard::ID_ADD_ACCOUNT, ab::SetupWizard::OnAddAccount)
END_EVENT_TABLE()


ab::SetupWizard::SetupWizard(wxFrame* frame)
{
    auto path = wxGetApp().mAsserts / "icons" / "wiztest.svg";

    Create(frame, wxID_ANY, "PharmaOffice - Setup pharmacy",
        wxBitmapBundle::FromSVGFile(path.string(), FromDIP(wxSize(116, 260))),
        wxDefaultPosition,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    pageSize = FromDIP(wxSize(600, 400));
    SetBitmapPlacement(wxWIZARD_TILE);
    SetBackgroundColour(*wxWHITE);
    SetSize(FromDIP(wxSize(700, 400)));

    wxIcon appIcon;
    appIcon.CopyFromBitmap(wxArtProvider::GetBitmap("pharmaofficeico"));
    SetIcon(appIcon);

    SetBitmapPlacement(wxWIZARD_VALIGN_CENTRE);
    SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);

    CreateFirstPage();
    CreateContactPage();
    CreateAddressPage();
    CreateBranchPage();
    CreateAddAccountPage();
    CreateSummaryPage();
}

void ab::SetupWizard::OnFinished(wxWizardEvent& evt)
{
    auto& app = wxGetApp();
    state = TransferDataFromWindow();
    if (state) {
        bool c = app.mPharmacyManager.CreatePharmacy();
        if(c)
           c = app.mPharmacyManager.CreateBranch();

        state = c;
    }
}

void ab::SetupWizard::OnAddAccount(wxCommandEvent& evt)
{
    //create something here
}

bool ab::SetupWizard::TransferDataFromWindow()
{
    auto& app = wxGetApp();
    try {
        auto format = [](std::string& str) {
            boost::trim(str);
            boost::to_lower(str);
         };

        app.mPharmacyManager.pharmacy.name = mPharmacyNameValue->GetValue().ToStdString();
        format(app.mPharmacyManager.pharmacy.name);

        app.mPharmacyManager.branch.name = mBranchNameValue->GetValue().ToStdString();
        format(app.mPharmacyManager.branch.name);

        app.mPharmacyManager.branch.type = static_cast<grape::branch_type>(mPharmacyTypeValue->GetSelection());

        app.mPharmacyManager.address.country = mCountryValue->GetValue().ToStdString();
        app.mPharmacyManager.address.state = mStateValue->GetValue().ToStdString();
        app.mPharmacyManager.address.lga = mLgaValue->GetValue().ToStdString();
        app.mPharmacyManager.address.street = mStreetValue->GetValue().ToStdString();
        app.mPharmacyManager.address.num = mNoValue->GetValue().ToStdString();


        format(app.mPharmacyManager.address.country);
        format(app.mPharmacyManager.address.state);
        format(app.mPharmacyManager.address.lga);
        format(app.mPharmacyManager.address.street);
        format(app.mPharmacyManager.address.num);
    }
    catch (const std::exception& exp) {
        spdlog::error(exp.what());
        wxMessageBox(exp.what(), "Error in data transfer", wxICON_ERROR | wxOK);

        return false;
    }
    return true;
}

void ab::SetupWizard::CreateSelectPage()
{
    mSelectPage = new wxWizardPageSimple(this);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxPanel* cp = new wxPanel(mSelectPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* bS2;
    bS2 = new wxBoxSizer(wxHORIZONTAL);

    auto TitleText = new wxStaticText(cp, wxID_ANY, wxT("What would you like to do?"), wxDefaultPosition, wxDefaultSize, 0);
    TitleText->Wrap(-1);
    TitleText->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    bS2->AddStretchSpacer(1);
    bS2->Add(TitleText, 0, wxEXPAND | wxALL, 5);
    bS2->AddStretchSpacer(1);
    cp->SetSizer(bS2);
    cp->Layout();

    bSizer1->Add(cp, 0, wxALL, 2);

    wxPanel* cp2 = new wxPanel(mSelectPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* bs3 = new wxBoxSizer(wxHORIZONTAL);
    wxArrayString choices;
    choices.push_back("Create a pharmacy"s);
    choices.push_back("Open a branch for a pharmacy"s);
    choices.push_back("New system in branch"s);
    wxRadioBox* rbox = new wxRadioBox(cp2, wxID_ANY, "Please select an option"s, wxDefaultPosition, wxDefaultSize,
    choices);
    rbox->Bind(wxEVT_RADIOBOX, [&](wxCommandEvent& evt) {
        select = evt.GetSelection();
        switch (select) {
        case 0:
        {
            mSelectPage->Chain(mFirstPage)
                .Chain(mBranchPage).Chain(mAddressPage).Chain(mAddAccountPage);
        }
            break;
        case 1:
            mSelectPage->Chain(mSelectPharmacyPage).Chain(mBranchPage).Chain(mAddressPage).
                Chain(mAddAccountPage);
            break;
        case 2:
            mSelectPage->Chain(mSelectBranchPage);
            break;
        default:
            break;
        }
     });

    bs3->AddStretchSpacer();
    bs3->Add(rbox, 1, wxALL, 2);
    bs3->AddStretchSpacer();

    cp2->SetSizer(bs3);
    cp2->Layout();

    bSizer1->Add(cp2, 0, wxALL, 2);

    mSelectPage->SetSizer(bSizer1);
    mSelectPage->Layout();

    mSelectPage->Chain(mFirstPage);
}

void ab::SetupWizard::CreateSelectPharmacy()
{
    mSelectPharmacyPage = new wxWizardPageSimple(this);
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mContactPage, wxID_ANY, wxT("Please select a pharmacy for this branch.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mContactPage, wxID_ANY, wxT(R"(Select your pharmacy that you want to add this branch to)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    
    wxSimplebook* mbook = new wxSimplebook(mSelectPharmacyPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    mActivityIndicator = new wxActivityIndicator(mbook, ID_ACTIVITY);
        
    mbook->AddPage(mActivityIndicator, "Waiting", false);
    
    mListCtrl = new wxDataViewListCtrl(mbook, ID_LISTCTRL);
    mListCtrl->AppendTextColumn("Name", wxDATAVIEW_CELL_INERT, 300);
    mListCtrl->AppendTextColumn("Address", wxDATAVIEW_CELL_INERT, 300);
    mListCtrl->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, [&](wxDataViewEvent& evt) {
        auto item = evt.GetItem();
        if (!item.IsOk()) return;

        size_t idx = reinterpret_cast<size_t>(item.GetID()) - 1;
        pharmacy = mPharmacies[idx];

        mSelectPharmacyPage->Chain(mBranchPage);
    }, ID_LISTCTRL);

    mbook->AddPage(mListCtrl, "Listctrl", true);

  
    bSizer1->Add(mbook, wxSizerFlags().Proportion(1).Expand().Border(2, wxALL));
    mSelectPharmacyPage->SetSizer(bSizer1);
    mSelectPharmacyPage->Layout();

  //  mSelectBranchPage->Chain(mAddAccountPage);
}

void ab::SetupWizard::CreateFirstPage()
{
    // a wizard page may be either an object of predefined class
    mFirstPage = new wxWizardPageSimple(this);
    mFirstPage->SetSize(pageSize);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    m_panel1 = new wxPanel(mFirstPage, wxID_ANY, wxDefaultPosition, pageSize, wxTAB_TRAVERSAL);
    m_panel1->SetBackgroundColour(*wxWHITE);
    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer(wxVERTICAL);

    mTitle = new wxStaticText(m_panel1, wxID_ANY, wxT("Welcome to PharmaOffice\n"), wxDefaultPosition, wxDefaultSize, 0);
    mTitle->Wrap(-1);
    mTitle->SetFont(wxFont(wxFontInfo(12).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer2->Add(mTitle, 0, wxTOP | wxLEFT, 5);


    mDescription = new wxStaticText(m_panel1, wxID_ANY, wxT(R"(PharmaOffice is a package designed to make the management of pharmacy simple and seamless.
Let's get started.)"), wxDefaultPosition, wxDefaultSize, 0);
    mDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    mDescription->Wrap(-1);

    bSizer2->Add(mDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    mPharamcyName = new wxStaticText(m_panel1, wxID_ANY, wxT("Pharmacy name"), wxDefaultPosition, wxDefaultSize, 0);
    mPharamcyName->Wrap(-1);
    bSizer2->Add(mPharamcyName, 0, wxEXPAND | wxALL, 5);

    mPharmacyNameValue = new wxTextCtrl(m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mPharmacyNameValue->SetMaxLength(250);
    mPharmacyNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
    bSizer2->Add(mPharmacyNameValue, 0, wxALL, 5);

    m_panel1->SetSizer(bSizer2);
    m_panel1->Layout();
    bSizer1->Add(m_panel1, 0, wxEXPAND | wxALL, 5);

    mFirstPage->SetSizer(bSizer1);
    mFirstPage->Layout();
    GetPageAreaSizer()->Add(mFirstPage);
}

void ab::SetupWizard::CreateContactPage()
{
    mContactPage = new wxWizardPageSimple(this);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mContactPage, wxID_ANY, wxT("Please enter the pharmacy contact details.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mContactPage, wxID_ANY, wxT(R"(PharmaOffice uses the contact information for receipts and reports, please fill in the information)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    mPhoneNo = new wxStaticText(mContactPage, wxID_ANY, wxT("Phone No"), wxDefaultPosition, wxDefaultSize, 0);
    mPhoneNo->Wrap(-1);
    bSizer1->Add(mPhoneNo, 0, wxEXPAND | wxALL, 5);

    mPhoneNoValue = new wxTextCtrl(mContactPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mPhoneNoValue->SetMaxLength(250);
    mPhoneNoValue->SetValidator(ab::RegexValidator(std::regex("(0|91)?[6-9][0-9]{9}"), "Invalid phone number"));
    bSizer1->Add(mPhoneNoValue, 0, wxALL, 5);

    mEmail = new wxStaticText(mContactPage, wxID_ANY, wxT("Email"), wxDefaultPosition, wxSize(450, -1), 0);
    mEmail->Wrap(-1);
    bSizer1->Add(mEmail, 0, wxEXPAND | wxALL, 5);

    mEmailValue = new wxTextCtrl(mContactPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mEmailValue->SetMaxLength(250);
    mEmailValue->SetValidator(ab::RegexValidator(std::regex(R"(^[\w-\.]+@([\w-]+\.)+[\w-]{2,4}$)"), "Invalid email address"));
    bSizer1->Add(mEmailValue, 0, wxALL, 5);

    mWebsiteText = new wxStaticText(mContactPage, wxID_ANY, wxT("Website"), wxDefaultPosition, wxSize(450, -1), 0);
    mWebsiteText->Wrap(-1);
    bSizer1->Add(mWebsiteText, 0, wxALL, 5);

    mWebsiteValue = new wxTextCtrl(mContactPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mWebsiteValue->SetMaxLength(250);
    bSizer1->Add(mWebsiteValue, 0, wxALL, 5);

    mContactPage->SetSizer(bSizer1);
    mContactPage->Layout();

    mFirstPage->Chain(mContactPage);
}

void ab::SetupWizard::CreateAddressPage()
{
    mAddressPage = new wxWizardPageSimple(this);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mAddressPage, wxID_ANY, wxT("Please enter the pharmacy address details.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mAddressPage, wxID_ANY, wxT(R"(PharmaOffice uses the address information for receipts and reports, please fill in the information)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    mCountyText = new wxStaticText(mAddressPage, wxID_ANY, wxT("Country"), wxDefaultPosition, wxDefaultSize, 0);
    mCountyText->Wrap(-1);
    bSizer1->Add(mCountyText, 0, wxALL, 5);

    mCountryValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mCountryValue->SetMaxLength(250);
    bSizer1->Add(mCountryValue, 0, wxALL, 5);

    mLgaText = new wxStaticText(mAddressPage, wxID_ANY, wxT("L.G.A"), wxDefaultPosition, wxDefaultSize, 0);
    mLgaText->Wrap(-1);
    bSizer1->Add(mLgaText, 0, wxALL, 5);

    mLgaValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mLgaValue->SetMaxLength(250);
    bSizer1->Add(mLgaValue, 0, wxALL, 5);

    mNoText = new wxStaticText(mAddressPage, wxID_ANY, wxT("No."), wxDefaultPosition, wxDefaultSize, 0);
    mNoText->Wrap(-1);
    bSizer1->Add(mNoText, 0, wxALL, 5);

    mNoValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mNoValue->SetMaxLength(250);
    mNoValue->SetValidator(wxTextValidator{ wxFILTER_NUMERIC });
    bSizer1->Add(mNoValue, 0, wxALL, 5);

    mStreetText = new wxStaticText(mAddressPage, wxID_ANY, wxT("Street"), wxDefaultPosition, wxDefaultSize, 0);
    mStreetText->Wrap(-1);
    bSizer1->Add(mStreetText, 0, wxALL, 5);

    mStreetValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mStreetValue->SetMaxLength(250);
    bSizer1->Add(mStreetValue, 0, wxALL, 5);

    mCityText = new wxStaticText(mAddressPage, wxID_ANY, wxT("City"), wxDefaultPosition, wxDefaultSize, 0);
    mCityText->Wrap(-1);
    bSizer1->Add(mCityText, 0, wxALL, 5);

    mCityValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mCityValue->SetMaxLength(250);
    bSizer1->Add(mCityValue, 0, wxALL, 5);

    mStateText = new wxStaticText(mAddressPage, wxID_ANY, wxT("State"), wxDefaultPosition, wxSize(450, -1), 0);
    mStateText->Wrap(-1);
    bSizer1->Add(mStateText, 0, wxALL, 5);

    mStateValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0);
    mStateValue->SetMaxLength(250);
    bSizer1->Add(mStateValue, 0, wxALL, 5);



    mAddressPage->SetSizer(bSizer1);
    mAddressPage->Layout();

    mContactPage->Chain(mAddressPage);
}

void ab::SetupWizard::CreateBranchPage()
{
    mBranchPage = new wxWizardPageSimple(this);
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mBranchPage, wxID_ANY, wxT("Please create a pharmacy branch.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mBranchPage, wxID_ANY, wxT(R"(PharmaOffice uses arranges your pharmacy into branches, this is to allow the data and information be shared across your pharmacies)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);


    mBranchName = new wxStaticText(mBranchPage, wxID_ANY, wxT("Branch name: "), wxDefaultPosition, wxDefaultSize, 0);
    mCountyText->Wrap(-1);
    bSizer1->Add(mBranchName, 0, wxALL, 5);

    mBranchNameValue = new wxTextCtrl(mBranchPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mBranchNameValue->SetMaxLength(250);
    mBranchNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
    bSizer1->Add(mBranchNameValue, 0, wxALL, 5);

    mPharmacyType = new wxStaticText(mBranchPage, wxID_ANY, wxT("Pharmacy type"), wxDefaultPosition, wxDefaultSize, 0);
    mPharmacyType->Wrap(-1);
    bSizer1->Add(mPharmacyType, 0, wxEXPAND | wxALL, 5);

    wxString mPharamcyTypeValueChoices[] = { wxT("COMMUNITY"), wxT("HOSPITAL"), wxT("INDUSTRY"), wxT("DRF"), wxT("EDUCATIONAL") };
    int mPharamcyTypeValueNChoices = sizeof(mPharamcyTypeValueChoices) / sizeof(wxString);
    mPharmacyTypeValue = new wxChoice(mBranchPage, wxID_ANY, wxDefaultPosition, wxSize(450, -1), mPharamcyTypeValueNChoices, mPharamcyTypeValueChoices, 0);
    mPharmacyTypeValue->SetSelection(0);
    mPharmacyTypeValue->Bind(wxEVT_CHOICE, [&](wxCommandEvent& evt) {
        int sel = evt.GetSelection();
        if (sel == wxNOT_FOUND) return;

     });
    mPharmacyTypeValue->Bind(wxEVT_PAINT, [=](wxPaintEvent& evt) {
        wxPaintDC dc(mPharmacyTypeValue);
        wxRect rect(0, 0, dc.GetSize().GetWidth(), dc.GetSize().GetHeight());


        dc.SetBrush(*wxWHITE);
        dc.SetPen(*wxGREY_PEN);
        dc.DrawRoundedRectangle(rect, 2.0f);
        dc.DrawBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_OTHER, FromDIP(wxSize(10, 10))), wxPoint(rect.GetWidth() - FromDIP(15), (rect.GetHeight() / 2) - FromDIP(5)));
        auto sel = mPharmacyTypeValue->GetStringSelection();
        if (!sel.IsEmpty()) {
            dc.DrawLabel(sel, rect, wxALIGN_CENTER);
        }
        });

    bSizer1->Add(mPharmacyTypeValue, 0, wxALL, 5);


    mBranchPage->SetSizer(bSizer1);
    mBranchPage->Layout();

    mAddressPage->Chain(mBranchPage);
}

void ab::SetupWizard::CreateAddAccountPage()
{
    mAddAccountPage = new wxWizardPageSimple(this);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mAddAccountPage, wxID_ANY, wxT("Please add an account for the pharmacy.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mAddAccountPage, wxID_ANY, wxT(R"(Please an a user account to the pharmacy.)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    btn = new wxButton(mAddAccountPage, ID_ADD_ACCOUNT);
    btn->SetBitmap(wxArtProvider::GetBitmap("add_task", wxART_OTHER, FromDIP(wxSize(16, 16))));
    btn->SetLabel("Add user account");
    btn->SetBackgroundColour(*wxWHITE);
    bSizer1->Add(btn, 0, wxALL, 5);


    mAddAccountPage->SetSizer(bSizer1);
    mAddAccountPage->Layout();

    mBranchPage->Chain(mAddAccountPage);
}

void ab::SetupWizard::CreateSummaryPage()
{
    mSummaryPage = new wxWizardPageSimple(this);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mSummaryPage, wxID_ANY, wxT("You have completed the pharmacy setup.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mSummaryPage, wxID_ANY, wxT(R"(Please click finish to start using PharmaOffice.)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);


    mSummaryPage->SetSizer(bSizer1);
    mSummaryPage->Layout();

    mAddAccountPage->Chain(mSummaryPage);
}

void ab::SetupWizard::CreateSelectBranchPage()
{
    mSelectBranchPage = new  wxWizardPageSimple(this);
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mSelectBranchPage, wxID_ANY, fmt::format("Please select a branch for {} pharmacy.\n", pharmacy.name), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, FromDIP(5));

    auto conDescription = new wxStaticText(mSelectBranchPage, wxID_ANY, wxT(R"(Select your pharmacy that you want to add this branch to)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, FromDIP(5));


    wxPanel* Entry = new wxPanel(mSelectBranchPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* sz = new wxBoxSizer(wxHORIZONTAL);

    sz->AddSpacer(FromDIP(10));

    mBranchIdEntry = new wxTextCtrl(Entry, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    mEnterBranchId = new wxButton(Entry, wxID_ANY, "Proceed", wxDefaultPosition, wxDefaultSize);
    sz->Add(mBranchIdEntry, wxSizerFlags().Proportion(1).Expand().Border(wxALL, FromDIP(2)));
    sz->Add(mEnterBranchId, wxSizerFlags().Proportion(0).Border(wxALL, FromDIP(2)));

    Entry->SetSizer(sz);
    Entry->Layout();

    auto handle =  [&](wxCommandEvent& evt) {
        auto text = mBranchIdEntry->GetValue().ToStdString();
        try {
            auto& app = wxGetApp();
            auto id = boost::lexical_cast<boost::uuids::uuid>(text);
            //how do we get the branch id
            //show a busy info ?
            auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
            grape::uid_t uid;
            boost::fusion::at_c<0>(uid) = id;

            grape::session::request_type::body_type::value_type body(grape::serial::get_size(uid), 0x00);
            grape::serial::write(boost::asio::buffer(body), uid);

            auto fut = sess->req(http::verb::get, "/pharmacy/getbranchesid", std::move(body));


            //how do I wait for this
            std::chrono::seconds elapsed = 0s;
            std::future_status st = fut.wait_for(1s);
            while (st != std::future_status::ready && elapsed < 30s) {
                //keep waiting ?


                st = fut.wait_for(1s);
                elapsed += 1s;
            }
            if (elapsed > 30s) {
                //display somethinh
                wxMessageBox("Request timeout, please try again", "Branch ID", wxICON_INFORMATION | wxOK);
                return;
            }

            auto resp = fut.get();
            if (resp.result() != http::status::ok) {
                throw std::logic_error(app.ParseServerError(resp));
            }

            auto& body = resp.body();
            auto&& [b, buf] = grape::serial::read<grape::branch>(boost::asio::buffer(body));
            branch = std::move(b);
            wxMessageBox(fmt::format("Added system portal to branch {}", branch.name), "Setup", wxICON_INFORMATION | wxOK);
        }
        catch (const boost::bad_lexical_cast& exp) {
            wxMessageBox(fmt::format("Invaid branch Id: {}", exp.what()), "Branch ID", wxICON_ERROR | wxOK);
        }
        catch (const std::exception& exp) {
            wxMessageBox(exp.what(), "Branch ID", wxICON_ERROR | wxOK);
        }
    };

    mBranchIdEntry->Bind(wxEVT_TEXT_ENTER, handle);
    mEnterBranchId->Bind(wxEVT_BUTTON, handle);

    bSizer1->Add(Entry, wxSizerFlags().Proportion(0).Border(wxALL, 2));

    mBranchBook = new wxSimplebook(mSelectBranchPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    
    wxPanel* panel = new wxPanel(mBranchBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* psz = new wxBoxSizer(wxVERTICAL);


    mBranchActivityIndicator = new wxActivityIndicator(panel, ID_ACTIVITY);
    auto d = new wxStaticText(panel, wxID_ANY, "Please wait...", wxDefaultPosition, wxDefaultSize, 0);

    psz->AddSpacer(FromDIP(5));
    psz->Add(mBranchActivityIndicator, wxSizerFlags().Proportion(1).CenterHorizontal().Expand().Border(wxALL, FromDIP(5)));
    psz->AddSpacer(FromDIP(5));
    psz->Add(d, wxSizerFlags().Proportion(0).CenterHorizontal().Border(wxALL, FromDIP(5)));


    mBranchBook->AddPage(panel, "Waiting", false);

    mBranchListCtrl = new wxDataViewListCtrl(mBranchBook, ID_LISTCTRL);
    mBranchListCtrl->AppendTextColumn("Name", wxDATAVIEW_CELL_INERT, 300);
    mBranchListCtrl->AppendTextColumn("Address", wxDATAVIEW_CELL_INERT, 300);
    mBranchListCtrl->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, [&](wxDataViewEvent& evt) {
        auto item = evt.GetItem();
        if (!item.IsOk()) return;

        size_t idx = reinterpret_cast<size_t>(item.GetID()) - 1;
        pharmacy = mPharmacies[idx];

    }, ID_LISTCTRL);

    mBranchBook->AddPage(mBranchListCtrl, "Listctrl", true);


    bSizer1->Add(mBranchBook, wxSizerFlags().Proportion(1).Expand().Border(wxALL, FromDIP(2)));
    mSelectBranchPage->SetSizer(bSizer1);
    mSelectBranchPage->Layout();
}

void ab::SetupWizard::LoadPharmacies()
{
    auto& app = wxGetApp();
    auto ph = app.mPharmacyManager.GetPharmacies();
    mPharmacies = std::move(boost::fusion::at_c<0>(ph));

    if (mListCtrl) {
        for (auto p : mPharmacies) {
            wxVector<wxVariant> mdata;
            mdata.push_back(p.name);
            

            mListCtrl->AppendItem(mdata);
        }
    }
}

void ab::SetupWizard::LoadBranches()
{
    if (pharmacy.id == boost::uuids::nil_uuid()) return;

    auto bh = wxGetApp().mPharmacyManager.GetBranches(pharmacy.id, 0, 100);
    mBranches = std::move(boost::fusion::at_c<0>(bh));

    if (mBranchListCtrl) {
        for (auto& b : mBranches) {
            wxVector<wxVariant> data;
            data.push_back(b.name);
           

            mBranchListCtrl->AppendItem(data);
        }
    }
}
