#include "SetupWizard.hpp"


BEGIN_EVENT_TABLE(ab::SetupWizard, wxWizard)
	EVT_WIZARD_FINISHED(wxID_ANY, ab::SetupWizard::OnFinished)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, ab::SetupWizard::OnPageChanging)
    EVT_WIZARD_PAGE_CHANGED(wxID_ANY, ab::SetupWizard::OnPageChanged)
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
   // SetBitmapPlacement(wxWIZARD_TILE);
    SetBackgroundColour(*wxWHITE);
    SetSize(FromDIP(wxSize(700, 400)));

    wxIcon appIcon;
    appIcon.CopyFromBitmap(wxArtProvider::GetBitmap("pharmaofficeico"));
    SetIcon(appIcon);

    SetBitmapPlacement(wxWIZARD_VALIGN_CENTRE);
    SetLayoutAdaptationMode(wxDIALOG_ADAPTATION_MODE_ENABLED);


    CreateSelectPage();
    CreateSelectPharmacy();
    CreateSelectBranchPage();
    CreateFirstPage();
    CreateContactPage();
    CreateAddressPage();
    CreateBranchPage();
    CreateAddAccountPage();
    CreateSummaryPage();
}

void ab::SetupWizard::OnFinished(wxWizardEvent& evt)
{
    state = TransferDataFromWindow();
    if (state) {
        CreateAppSettings();
    }
    else {
        wxMessageBox("Failed to setup PharmaOffice", "Setup", wxICON_ERROR | wxOK);
    }
}

void ab::SetupWizard::OnPageChanging(wxWizardEvent& evt)
{
    auto page = evt.GetPage();
    if (page == mSelectPharmacyPage && evt.GetDirection()) {
        if (pharmacy.id.is_nil())
        {
            wxMessageBox("No pharmacy selected", "Setup", wxICON_ERROR | wxOK);
            evt.Veto();
        }
    }
    if (page == mSelectBranchPage && evt.GetDirection()) {
        if (branch.id.is_nil()) {
            wxMessageBox("No branch selected", "Setup", wxICON_ERROR | wxOK);
            evt.Veto();
        }
    }
}

void ab::SetupWizard::OnPageChanged(wxWizardEvent& evt)
{
    if (!evt.GetDirection()) return;
    auto page = evt.GetPage();
    if (page == mSelectPharmacyPage) {
        mActivityIndicator->Start();
        mLoadPharmWait = std::async(std::launch::async, std::bind_front(&ab::SetupWizard::LoadPharmacies, this));   
    }
    else if (page == mSelectBranchPage) {
        mBranchActivityIndicator->Start();
        mLoadBranchWait = std::async(std::launch::async, &ab::SetupWizard::LoadBranches, this);
    }
    else if (page == mSummaryPage) {
       //if we change to summary we have completed collecting data
        mSetupStatus = true;
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
#ifndef  _DEBUG
        wxIcon cop;
        cop.CopyFromBitmap(wxArtProvider::GetBitmap("doctor"));
        wxBusyInfo info
        (
             wxBusyInfoFlags()
            .Parent(this)
            .Icon(cop)
            .Title("Creating pharmacy")
            .Text("Please wait...")
            .Foreground(*wxBLACK)
            .Background(*wxWHITE)
           // .Transparency(4 * wxALPHA_OPAQUE / 5)
        );
#endif // ! _DEBUG


        //set address regardless
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

        switch (stype)
        {
        case setup_type::create_pharmacy:
        {
            app.mPharmacyManager.pharmacy.name = mPharmacyNameValue->GetValue().ToStdString();
            format(app.mPharmacyManager.pharmacy.name);

            app.mPharmacyManager.branch.name = mBranchNameValue->GetValue().ToStdString();
            format(app.mPharmacyManager.branch.name);

            app.mPharmacyManager.branch.type = static_cast<grape::branch_type>(mPharmacyTypeValue->GetSelection());

            js::json info = js::json::object();
            js::json contact = js::json::object();

            contact["phone"] = mPhoneNoValue->GetValue().ToStdString();
            contact["email"] = mEmailValue->GetValue().ToStdString();
            contact["website"] = mWebsiteValue->GetValue().ToStdString();


            info["contact"] = contact;

            app.mPharmacyManager.pharmacy.info = info.dump();
            if (!app.mPharmacyManager.CreatePharmacy() ||
                !app.mPharmacyManager.CreateBranch()) {
                return false;
            }
        }
        break;
        case setup_type::create_branch:
        {
            app.mPharmacyManager.branch.pharmacy_id = pharmacy.id;
            
            app.mPharmacyManager.branch.name = mBranchNameValue->GetValue().ToStdString();
            format(app.mPharmacyManager.branch.name);

            app.mPharmacyManager.branch.type = static_cast<grape::branch_type>(mPharmacyTypeValue->GetSelection());
            app.mPharmacyManager.CreateBranch();
        }
        break;
        case setup_type::create_branch_system:
        {
            app.mPharmacyManager.pharmacy = pharmacy;
            app.mPharmacyManager.branch = branch;
            return true;
        }
        break;
        default:
            return false;
        }
    }
    catch (const std::exception& exp) {
        spdlog::error(exp.what());
        wxMessageBox(exp.what(), fmt::format("Error in data transfer: {}", exp.what()), wxICON_ERROR | wxOK);

        return false;
    }
    return true;
}

bool ab::SetupWizard::CreateAppSettings()
{
    auto& app = wxGetApp();
    js::json mSettings = js::json::object();
    try {
        auto fpath = fs::current_path() / ".data"s;
        if (!fs::is_directory(fpath)) {
            fs::create_directory(fpath);
        }

        fpath /= "settings.json"s;
        if (fs::exists(fpath)) return true;
 
        mSettings["pharmacy_id"] = boost::lexical_cast<std::string>(app.mPharmacyManager.pharmacy.id);
        mSettings["pharmacy_name"] = app.mPharmacyManager.pharmacy.name;
        mSettings["branch_id"] = boost::lexical_cast<std::string>(app.mPharmacyManager.branch.id);
        mSettings["branch_name"] = app.mPharmacyManager.branch.name;
       // mSettings["branch_type"] = app.mPharmacyManager.branch.type;
        mSettings["address_id"] = app.mPharmacyManager.address.id;


        std::fstream file(fpath, std::ios::out);
        if (!file.is_open()) throw std::logic_error("failed to create settings file");
        file << mSettings.dump();
        file.close();
        return true;
    }
    catch (const std::exception& err) {
        spdlog::error(err.what());
        return false;
    }

}

void ab::SetupWizard::CreateSelectPage()
{
    mSelectPage = new wxWizardPageSimple(this);
    mSelectPage->SetSize(pageSize);

    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxPanel* cp = new wxPanel(mSelectPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* bS2;
    bS2 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(cp, wxID_ANY, wxT("Welcome to PharmaOffice setup.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bS2->AddSpacer(FromDIP(5));
    bS2->Add(contTitle, 0, wxTOP | wxLEFT, 5);
        

    auto TitleText = new wxStaticText(cp, wxID_ANY, wxT("What would you like to do?"), wxDefaultPosition, wxDefaultSize, 0);
    TitleText->Wrap(-1);
    TitleText->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    
    //bS2->AddSpacer(FromDIP(5));
    bS2->Add(TitleText, 0, wxEXPAND | wxALL, FromDIP(5));
    
    
    cp->SetSizer(bS2);
    cp->Layout();

    bSizer1->Add(cp, 0, wxALL, 2);

    wxPanel* cp2 = new wxPanel(mSelectPage, wxID_ANY, wxDefaultPosition, pageSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* bs3 = new wxBoxSizer(wxHORIZONTAL);
    wxArrayString choices;
    choices.push_back("Create a pharmacy"s);
    choices.push_back("Open a branch for a pharmacy"s);
    choices.push_back("New system in branch"s);
    wxRadioBox* rbox = new wxRadioBox(cp2, wxID_ANY, "Please select an option"s, wxDefaultPosition, wxDefaultSize,
    choices, 0, wxRA_SPECIFY_ROWS);
    rbox->Bind(wxEVT_RADIOBOX, [&](wxCommandEvent& evt) {
        select = evt.GetSelection();
        switch (select) {
        case 0:
        {
            mSelectPage->Chain(mFirstPage)
                .Chain(mContactPage).Chain(mAddressPage).Chain(mBranchPage).Chain(mAddAccountPage).Chain(mSummaryPage);
            stype = setup_type::create_pharmacy;
        }
            break;
        case 1:
            mSelectPage->Chain(mSelectPharmacyPage).Chain(mBranchPage).Chain(mAddressPage).
                Chain(mAddAccountPage).Chain(mSummaryPage);
            stype = setup_type::create_branch;
            break;
        case 2:
            mSelectPage->Chain(mSelectPharmacyPage).Chain(mSelectBranchPage).Chain(mSummaryPage);
            stype = setup_type::create_branch_system;
            break;
        default:
            break;
        }
        mSelectPage->Refresh();
     });

    bs3->Add(rbox, 0, wxALL, 2);

    cp2->SetSizer(bs3);
    cp2->Layout();

    bSizer1->Add(cp2, wxSizerFlags().Expand().Proportion(0).Border(wxALL, FromDIP(2)));

    mSelectPage->SetSizer(bSizer1);
    mSelectPage->Layout();

    GetPageAreaSizer()->Add(mSelectPage);
}

void ab::SetupWizard::CreateSelectPharmacy()
{
    auto& app = wxGetApp();
    mSelectPharmacyPage = new wxWizardPageSimple(this);
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    auto contTitle = new wxStaticText(mSelectPharmacyPage, wxID_ANY, wxT("Please select a pharmacy for this branch.\n"), wxDefaultPosition, wxDefaultSize, 0);
    contTitle->Wrap(-1);
    contTitle->SetFont(wxFont(wxFontInfo(10).Bold().AntiAliased().Family(wxFONTFAMILY_SWISS)));

    bSizer1->Add(contTitle, 0, wxTOP | wxLEFT, 5);

    auto conDescription = new wxStaticText(mSelectPharmacyPage, wxID_ANY, wxT(R"(Enter pharmacy id)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription->Wrap(-1);

    bSizer1->Add(conDescription, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    wxPanel* Entry = new wxPanel(mSelectPharmacyPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxBoxSizer* sz = new wxBoxSizer(wxHORIZONTAL);

    sz->AddSpacer(FromDIP(10));

    mPharmacyIdEntry = new wxTextCtrl(Entry, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), wxTE_PROCESS_ENTER);
    mEnterPharmacyId = new wxButton(Entry, wxID_ANY, "Proceed", wxDefaultPosition, wxDefaultSize);

    mPharmacyIdEntry->SetHint("aaaa-bbbb-cccc-dddd"s);

    sz->Add(mPharmacyIdEntry, wxSizerFlags().Proportion(1).Expand().Border(wxTOP | wxBOTTOM | wxRIGHT, FromDIP(2)));
    sz->Add(mEnterPharmacyId, wxSizerFlags().Proportion(0).Border(wxALL, FromDIP(2)));

    Entry->SetSizer(sz);
    Entry->Layout();

    auto handle = [&](wxCommandEvent& evt) {
        auto text = mPharmacyIdEntry->GetValue().ToStdString();
        try {
            if (text.empty()) return;

            auto& app = wxGetApp();
            auto id = boost::lexical_cast<boost::uuids::uuid>(text);
            //how do we get the pharmacy id
            //show a busy info ?
            auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
            grape::uid_t uid;
            boost::fusion::at_c<0>(uid) = id;

            grape::session::request_type::body_type::value_type body(grape::serial::get_size(uid), 0x00);
            grape::serial::write(boost::asio::buffer(body), uid);

            auto fut = sess->req(http::verb::get, "/pharmacy/getpharmacybyid", std::move(body));


            //how do I wait for this
            std::chrono::seconds elapsed = 0s;
            std::future_status st = fut.wait_for(1s);
            while (st != std::future_status::ready && elapsed < 30s) {
                //keep waiting ?
                wxYield(); // thinking of this is a valid thing to do ?

                st = fut.wait_for(1s);
                elapsed += 1s;
            }
            if (elapsed > 30s) {
                //display somethinh
                wxMessageBox("Request timeout, please try again", "Set up", wxICON_INFORMATION | wxOK);
                return;
            }

            auto resp = fut.get();
            if (resp.result() != http::status::ok) {
                throw std::logic_error(app.ParseServerError(resp));
            }

            auto& body2 = resp.body();
            auto&& [b, buf] = grape::serial::read<grape::pharmacy>(boost::asio::buffer(body2));
            pharmacy = std::move(b);
            wxMessageBox(fmt::format("Added system portal to pharmacy {}", pharmacy.name), "Setup", wxICON_INFORMATION | wxOK);
        }
        catch (const boost::bad_lexical_cast& exp) {
            wxMessageBox(fmt::format("Invaid Pharmacy Id: {}", exp.what()), "Setup", wxICON_ERROR | wxOK);
        }
        catch (const std::exception& exp) {
            wxMessageBox(exp.what(), "Setup", wxICON_ERROR | wxOK);
            mPharmacyIdEntry->Clear();
        }

        };


    bSizer1->Add(Entry, wxSizerFlags().Proportion(0).Expand().Border(wxALL, 2));

    bSizer1->AddSpacer(FromDIP(10));

    auto conDescription2 = new wxStaticText(mSelectPharmacyPage, wxID_ANY, wxT(R"(Select the pharmacy you want to add this branch to)"), wxDefaultPosition, wxDefaultSize, 0);
    conDescription2->SetFont(wxFont(wxFontInfo().AntiAliased().Family(wxFONTFAMILY_SWISS)));
    conDescription2->Wrap(-1);

    bSizer1->Add(conDescription2, 0, wxTOP | wxBOTTOM | wxLEFT, 5);

    mPharmBook = new wxSimplebook(mSelectPharmacyPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    wxPanel* waitPanel = nullptr;
    std::tie(waitPanel, mActivityIndicator) = app.CreateWaitPanel(mPharmBook, "Please wait..."s);

    mPharmBook->AddPage(waitPanel, "Waiting", true);
    
    mListCtrl = new wxDataViewListCtrl(mPharmBook, ID_LISTCTRL);
    mListCtrl->AppendTextColumn("Name", wxDATAVIEW_CELL_INERT, 300);
    mListCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [&](wxDataViewEvent& evt) {
        auto item = evt.GetItem();
        if (!item.IsOk()) return;

        size_t idx = reinterpret_cast<size_t>(item.GetID()) - 1;
        pharmacy = mPharmacies[idx];

    }, ID_LISTCTRL);

    mPharmBook->AddPage(mListCtrl, "Listctrl", false);

    wxPanel* emptyPanel = nullptr;
    wxButton* btn = nullptr;
    std::tie(emptyPanel, mPharmEmptyText, btn) = app.CreateEmptyPanel(mPharmBook, "No internet connection", wxART_ERROR, wxART_MESSAGE_BOX);

    btn->SetBitmap(wxArtProvider::GetBitmap(wxART_REFRESH, wxART_BUTTON, FromDIP(wxSize(16, 16))));
    btn->SetLabel("Try again"s);
    btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
        mPharmBook->SetSelection(0);
        mActivityIndicator->Start();

        mLoadPharmWait = std::async(std::launch::async, std::bind_front(&ab::SetupWizard::LoadPharmacies, this));
    });

    mPharmBook->AddPage(emptyPanel, "No connection panel", false);

    bSizer1->Add(mPharmBook, wxSizerFlags().Proportion(1).Expand().Border(wxALL, FromDIP(2)));
    mSelectPharmacyPage->SetSizer(bSizer1);
    mSelectPharmacyPage->Layout();
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

    mPharmacyNameValue = new wxTextCtrl(m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mPharmacyNameValue->SetMaxLength(250);
    mPharmacyNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
    bSizer2->Add(mPharmacyNameValue, 0, wxALL, 5);

    m_panel1->SetSizer(bSizer2);
    m_panel1->Layout();
    bSizer1->Add(m_panel1, 0, wxEXPAND | wxALL, 5);

    mFirstPage->SetSizer(bSizer1);
    mFirstPage->Layout();

    mSelectPage->Chain(mFirstPage);
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

    mPhoneNoValue = new wxTextCtrl(mContactPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mPhoneNoValue->SetMaxLength(250);
    mPhoneNoValue->SetValidator(ab::RegexValidator(std::regex("(0|91)?[6-9][0-9]{9}"), "Invalid phone number"));
    bSizer1->Add(mPhoneNoValue, 0, wxALL, 5);

    mEmail = new wxStaticText(mContactPage, wxID_ANY, wxT("Email"), wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mEmail->Wrap(-1);
    bSizer1->Add(mEmail, 0, wxEXPAND | wxALL, 5);

    mEmailValue = new wxTextCtrl(mContactPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mEmailValue->SetMaxLength(250);
    mEmailValue->SetValidator(ab::RegexValidator(std::regex(R"(^[\w-\.]+@([\w-]+\.)+[\w-]{2,4}$)"), "Invalid email address"));
    bSizer1->Add(mEmailValue, 0, wxALL, 5);

    mWebsiteText = new wxStaticText(mContactPage, wxID_ANY, wxT("Website"), wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mWebsiteText->Wrap(-1);
    bSizer1->Add(mWebsiteText, 0, wxALL, 5);

    mWebsiteValue = new wxTextCtrl(mContactPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mWebsiteValue->SetMaxLength(250);
    mWebsiteValue->SetValidator(ab::RegexValidator(std::regex(R"(^(https?:\/\/)?([a-zA-Z0-9-]+\.)+[a-zA-Z]{2,}(:[0-9]{1,5})?(\/[^\s]*)?$)"), "Invalid website address"));
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

    mCountryValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mCountryValue->SetMaxLength(250);
    bSizer1->Add(mCountryValue, 0, wxALL, 5);

    mLgaText = new wxStaticText(mAddressPage, wxID_ANY, wxT("L.G.A"), wxDefaultPosition, wxDefaultSize, 0);
    mLgaText->Wrap(-1);
    bSizer1->Add(mLgaText, 0, wxALL, 5);

    mLgaValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mLgaValue->SetMaxLength(250);
    bSizer1->Add(mLgaValue, 0, wxALL, 5);

    mNoText = new wxStaticText(mAddressPage, wxID_ANY, wxT("No."), wxDefaultPosition, wxDefaultSize, 0);
    mNoText->Wrap(-1);
    bSizer1->Add(mNoText, 0, wxALL, 5);

    mNoValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mNoValue->SetMaxLength(250);
    mNoValue->SetValidator(wxTextValidator{ wxFILTER_NUMERIC });
    bSizer1->Add(mNoValue, 0, wxALL, 5);

    mStreetText = new wxStaticText(mAddressPage, wxID_ANY, wxT("Street"), wxDefaultPosition, wxDefaultSize, 0);
    mStreetText->Wrap(-1);
    bSizer1->Add(mStreetText, 0, wxALL, 5);

    mStreetValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mStreetValue->SetMaxLength(250);
    bSizer1->Add(mStreetValue, 0, wxALL, 5);

    mCityText = new wxStaticText(mAddressPage, wxID_ANY, wxT("City"), wxDefaultPosition, wxDefaultSize, 0);
    mCityText->Wrap(-1);
    bSizer1->Add(mCityText, 0, wxALL, 5);

    mCityValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mCityValue->SetMaxLength(250);
    bSizer1->Add(mCityValue, 0, wxALL, 5);

    mStateText = new wxStaticText(mAddressPage, wxID_ANY, wxT("State"), wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
    mStateText->Wrap(-1);
    bSizer1->Add(mStateText, 0, wxALL, 5);

    mStateValue = new wxTextCtrl(mAddressPage, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), 0);
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
    auto& app = wxGetApp();
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

    mBranchIdEntry = new wxTextCtrl(Entry, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(450, -1)), wxTE_PROCESS_ENTER);
    mEnterBranchId = new wxButton(Entry, wxID_ANY, "Proceed", wxDefaultPosition, wxDefaultSize);
   
    mBranchIdEntry->SetHint("aaaa-bbbb-cccc-dddd"s);

    sz->Add(mBranchIdEntry, wxSizerFlags().Proportion(1).Expand().Border(wxALL, FromDIP(2)));
    sz->Add(mEnterBranchId, wxSizerFlags().Proportion(0).Border(wxALL, FromDIP(2)));

    Entry->SetSizer(sz);
    Entry->Layout();

    auto handle =  [&](wxCommandEvent& evt) {
        auto text = mBranchIdEntry->GetValue().ToStdString();
        try {
            if (text.empty()) return;

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
                wxYield(); // thinking of this is a valid thing to do ?

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

            auto& body2 = resp.body();
            auto&& [b, buf] = grape::serial::read<grape::branch>(boost::asio::buffer(body2));
            branch = std::move(b);
            wxMessageBox(fmt::format("Added system portal to branch {}", branch.name), "Setup", wxICON_INFORMATION | wxOK);
        }
        catch (const boost::bad_lexical_cast& exp) {
            wxMessageBox(fmt::format("Invaid branch Id: {}", exp.what()), "Branch ID", wxICON_ERROR | wxOK);
        }
        catch (const std::exception& exp) {
            wxMessageBox(exp.what(), "Branch ID", wxICON_ERROR | wxOK);
            mBranchIdEntry->Clear();
        }
    };

    mBranchIdEntry->Bind(wxEVT_TEXT_ENTER, handle);
    mEnterBranchId->Bind(wxEVT_BUTTON, handle);

    bSizer1->Add(Entry, wxSizerFlags().Proportion(0).Expand().Border(wxALL, 2));

    mBranchBook = new wxSimplebook(mSelectBranchPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
    
    wxPanel* panel = nullptr;
    std::tie(panel, mBranchActivityIndicator) = app.CreateWaitPanel(mBranchBook, "Please wait...");

    mBranchBook->AddPage(panel, "Waiting", true);

    mBranchListCtrl = new wxDataViewListCtrl(mBranchBook, ID_LISTCTRL);
    mBranchListCtrl->AppendTextColumn("Name", wxDATAVIEW_CELL_INERT, 300);
    mBranchListCtrl->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [&](wxDataViewEvent& evt) {
        auto item = evt.GetItem();
        if (!item.IsOk()) return;

        size_t idx = reinterpret_cast<size_t>(item.GetID()) - 1;
        branch = mBranches[idx];

    }, ID_LISTCTRL);

    mBranchBook->AddPage(mBranchListCtrl, "Listctrl", false);

    wxPanel* panel2 = nullptr;
    wxButton* btn = nullptr;
    std::tie(panel2, mBranchEmptyText, btn) = app.CreateEmptyPanel(mBranchBook, "No internet connection", wxART_ERROR, wxART_MESSAGE_BOX);

    btn->SetBitmap(wxArtProvider::GetBitmap(wxART_REFRESH, wxART_BUTTON, FromDIP(wxSize(16, 16))));
    btn->SetLabel("Try again"s);
    btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& evt) {
        mBranchBook->SetSelection(0);
        mBranchActivityIndicator->Start();
        mLoadBranchWait = std::async(std::launch::async, std::bind_front(&ab::SetupWizard::LoadBranches, this));
    });


    mBranchBook->AddPage(panel2, "Empty", false);

    bSizer1->Add(mBranchBook, wxSizerFlags().Proportion(1).Expand().Border(wxALL, FromDIP(2)));
    mSelectBranchPage->SetSizer(bSizer1);
    mSelectBranchPage->Layout();
}

void ab::SetupWizard::LoadPharmacies()
{
    auto& app = wxGetApp();
    if (mListCtrl->GetItemCount() > 0)
        mListCtrl->DeleteAllItems();

    auto ph = app.mPharmacyManager.GetPharmacies();
    mPharmacies = std::move(boost::fusion::at_c<0>(ph));
    if (mPharmacies.empty()){
        mPharmEmptyText->SetLabel("No pharmcies avaliable");
        mActivityIndicator->Stop();
        mPharmBook->SetSelection(2);
        return;
    }
    

    if (mListCtrl) {
        for (auto p : mPharmacies) {
            wxVector<wxVariant> mdata;
            mdata.push_back(p.name);
            

            mListCtrl->AppendItem(mdata);
        }
        mActivityIndicator->Stop();
        mPharmBook->SetSelection(1);
    }

}

void ab::SetupWizard::LoadBranches()
{
    if (pharmacy.id.is_nil()) return;
    if (mBranchListCtrl->GetItemCount() > 0)
        mBranchListCtrl->DeleteAllItems();


    auto bh = wxGetApp().mPharmacyManager.GetBranches(pharmacy.id, 0, 100);
    mBranches = std::move(boost::fusion::at_c<0>(bh));
    if (mBranches.empty()) {
        mBranchEmptyText->SetLabel("No branches avaliable");
        mBranchBook->SetSelection(2);
        mBranchActivityIndicator->Stop();
        return;
    }

    if (mBranchListCtrl) {
        for (auto& b : mBranches) {
            wxVector<wxVariant> data;
            data.push_back(b.name);
           

            mBranchListCtrl->AppendItem(data);
        }
        mBranchActivityIndicator->Stop();
        mBranchBook->SetSelection(1);
    }
}
