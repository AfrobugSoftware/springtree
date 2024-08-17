#include "MainFrame.hpp"
#include "Application.hpp"
BEGIN_EVENT_TABLE(ab::MainFrame, wxFrame)
	EVT_MENU(ab::MainFrame::ID_ABOUT, ab::MainFrame::OnAbout)
END_EVENT_TABLE()


ab::MainFrame::MainFrame()
{
}

ab::MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size)
 : wxFrame(parent, id, "PharmaOffice - Enterprice", position, size), mManager(this, ab::AuiTheme::AUIMGRSTYLE){
	SetSize(FromDIP(size));
	SetupAuiTheme();
	CreateMenubar();

	mPager = new wxSimplebook(this, ID_PAGER);
	CreateModules();
	
	CreateWelcomePage();
	CreateWorkspace();
	CreateImageList();

	mPager->AddPage(mWelcomePage, "Welcome", true);
	mPager->AddPage(mWorkspace, "Workspace", false);
	mManager.AddPane(mPager, wxAuiPaneInfo().Name("Pager").CaptionVisible(false).CenterPane().Show());


	mManager.Update();
	SetIcon(wxGetApp().mAppIcon);
}

ab::MainFrame::~MainFrame()
{
	delete mImageList;
	mImageList = nullptr;

	mManager.UnInit();
}

void ab::MainFrame::SetupAuiTheme()
{
	auto artProvider = mManager.GetArtProvider();
	//pass the art provider to a place where it gets its settings from?
	ab::AuiTheme::Update(artProvider);
	ab::AuiTheme::Register(std::bind_front(&ab::MainFrame::OnAuiThemeChange, this));
}

void ab::MainFrame::CreateMenubar()
{
	constexpr const size_t MenuCount = 8;
	auto Menus = std::array<wxMenu*, MenuCount>{
		new wxMenu,
		new wxMenu,
		new wxMenu,
		new wxMenu,
		new wxMenu,
		new wxMenu,
		new wxMenu,
		new wxMenu
	};

	auto MenuTitle = std::array<wxString, MenuCount>{
		"Accounts",
		"Pharmacy",
		"Products",
		"Formulary",
		"Prescriptions",
		"View",
		"Extentions",
		"Help"
	};


	wxMenuBar* bar = new wxMenuBar(MenuCount, Menus.data(), MenuTitle.data());
	SetMenuBar(bar);
}

void ab::MainFrame::CreateToolbar()
{

}

void ab::MainFrame::CreateModules()
{
	mModules = new ab::Modules(this, ID_MODULE, wxDefaultPosition, FromDIP(wxSize(548, -1)), wxTAB_TRAVERSAL);
	mManager.AddPane(mModules, wxAuiPaneInfo().Name("Modules").CaptionVisible(false).Left().PaneBorder().BottomDockable(false).Floatable(false).TopDockable(false).Show());

	ab::mod mod;
	mod.callback = std::bind_front(&ab::MainFrame::OnModuleActivated, this);
	mod.win = nullptr;
	mod.name = "Products";
	mod.img = 0;
	mod.id = mModules->mProducts;
	mModules->Add(std::move(mod));

	mod = ab::mod{};
	mod.callback = std::bind_front(&ab::MainFrame::OnModuleActivated, this);
	mod.win = nullptr;
	mod.name = "Sales";
	mod.img = 1;
	mod.id = mModules->mProducts;
	mModules->Add(std::move(mod));
}

void ab::MainFrame::CreateWorkspace()
{
	mWorkspace = new ab::Workspace(mPager, ID_WORKSPACE, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	mWorkspace->notifsignal.connect(std::bind_front(&ab::MainFrame::OnWorkspaceNotif, this));
}

void ab::MainFrame::CreateWelcomePage()
{
	auto& app = wxGetApp();
	auto cap = [&](const std::string& name) -> std::string {
		std::string ret = name;
		*ret.begin() = std::toupper(*ret.begin());
		return ret;
	};


	mWelcomePage = new wxPanel(mPager, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	mWelcomePage->SetBackgroundColour(*wxWHITE);
	mWelcomePage->SetDoubleBuffered(true);
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	wxPanel* m5 = new wxPanel(mWelcomePage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxHORIZONTAL);


	bSizer8->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	wxPanel* m7 = new wxPanel(m5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer(wxVERTICAL);


	bSizer9->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	auto today = std::chrono::system_clock::now();
	std::string todayTime = std::format("{:%H:%M}", today);
	std::chrono::sys_days dt = std::chrono::time_point_cast<std::chrono::sys_days::duration>(today);
	std::chrono::year_month_day ymd{ dt };
	std::chrono::weekday wk{ dt };


	std::stringstream os;
	os << fmt::format("{:%d} {}, {} {:%y}", dt, dayNames[wk.c_encoding()],
		monthNames[(std::uint32_t)ymd.month() - 1], dt);

	time1 = new wxStaticText(m7, wxID_ANY, todayTime, wxDefaultPosition, wxDefaultSize, 0);
	time1->SetFont(wxFontInfo(64).AntiAliased().Family(wxFONTFAMILY_SWISS));
	time1->Wrap(-1);
	time1->SetDoubleBuffered(true);
	bSizer9->Add(time1, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, FromDIP(5));


	date1 = new wxStaticText(m7, wxID_ANY, os.str(), wxDefaultPosition, wxDefaultSize, 0);
	date1->SetFont(wxFontInfo(12).AntiAliased().Family(wxFONTFAMILY_SWISS));
	date1->Wrap(-1);
	date1->SetDoubleBuffered(true);
	bSizer9->Add(date1, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, FromDIP(5));

	pharmName = new wxStaticText(m7, wxID_ANY, fmt::format("Welcome to {}", cap(app.mPharmacyManager.pharmacy.name)), wxDefaultPosition, wxDefaultSize, 0);
	pharmName->Wrap(-1);
	bSizer9->Add(pharmName, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, FromDIP(5));


	bSizer9->AddSpacer(FromDIP(20));

	wxPanel* tt = new wxPanel(m7, wxID_ANY);
	auto bsz = new wxBoxSizer(wxHORIZONTAL);

	mSelectList = new wxListCtrl(tt, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(340, 300)), wxLC_ICON | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE | wxFULL_REPAINT_ON_RESIZE | wxLC_EDIT_LABELS | wxNO_BORDER);
	CreateSelectList();

	bsz->AddStretchSpacer();
	bsz->AddSpacer(FromDIP(35));
	bsz->Add(mSelectList, 1, wxALL | wxEXPAND | wxCENTER, FromDIP(5));
	bsz->AddStretchSpacer();

	tt->SetSizer(bsz);
	bsz->Fit(tt);

	bSizer9->Add(tt, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, FromDIP(10));

	bSizer9->Add(0, 0, 1, wxEXPAND, FromDIP(5));


	m7->SetSizer(bSizer9);
	m7->Layout();
	bSizer9->Fit(m7);
	bSizer8->Add(m7, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));


	bSizer8->Add(0, 0, 1, wxEXPAND, FromDIP(5));


	m5->SetSizer(bSizer8);
	m5->Layout();
	bSizer8->Fit(m5);
	bSizer6->Add(m5, 1, wxEXPAND | wxALL, FromDIP(5));


	mWelcomePage->SetSizer(bSizer6);
	mWelcomePage->Layout();
}

void ab::MainFrame::CreateSelectList()
{
	wxImageList* imagelist = new wxImageList(48, 48);
	imagelist->Add(wxArtProvider::GetBitmap("supplement-bottle"));
	imagelist->Add(wxArtProvider::GetBitmap("checkout"));
	imagelist->Add(wxArtProvider::GetBitmap("doctor"));
	imagelist->Add(wxArtProvider::GetBitmap("prescription"));

	mSelectList->AssignImageList(imagelist, wxIMAGE_LIST_NORMAL);

	wxListItem item;
	item.SetId(0);
	item.SetText("Products");
	item.SetImage(0);
	item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT);

	mSelectList->InsertItem(item);

	item.SetId(1);
	item.SetText("Sales");
	item.SetImage(1);
	item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT);

	mSelectList->InsertItem(item);


	item.SetId(2);
	item.SetText("Patients");
	item.SetImage(2);
	item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT);

	mSelectList->InsertItem(item);

	item.SetId(3);
	item.SetText("Prescriptions");
	item.SetImage(3);
	item.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT);

	mSelectList->InsertItem(item);
	mSelectList->Bind(wxEVT_LIST_ITEM_ACTIVATED, std::bind_front(&ab::MainFrame::OnWelcomePageSelect, this));
}

void ab::MainFrame::CreateImageList()
{
	mImageList = new wxImageList(FromDIP(16), FromDIP(16));

	mImageList->Add(wxArtProvider::GetBitmap("store_tree", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("payments", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("heart_plus", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("shopping_bag", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("home", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("bar_chart", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("inventory", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("menu_book", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("monitoring", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("sprint", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("cases", wxART_OTHER, FromDIP(wxSize(16, 16))));
	mImageList->Add(wxArtProvider::GetBitmap("account_circle", wxART_OTHER, FromDIP(wxSize(16, 16))));

	mWorkspace->SetImageList(mImageList);
	mModules->SetImageList(mImageList);
}

void ab::MainFrame::OnWelcomePageSelect(wxListEvent& evt)
{
	int sel = evt.GetSelection();
	if (sel == wxNOT_FOUND) return;

	switch (sel)
	{
	case 0:
		mModules->ActivateModule(mModules->mProducts);
		break;
	case 1:
		mModules->ActivateModule(mModules->mSales);
		break;
	case 2:
		mModules->ActivateModule(mModules->mPaitents);
		break;
	case 3:
		mModules->ActivateModule(mModules->mPrescriptions);
		break;
	default:
		break;
	}
}

void ab::MainFrame::OnAbout(wxCommandEvent& evt)
{
	wxAboutDialogInfo info;
	info.SetName(wxT("PharmaOffice"));
	info.SetVersion(wxGetApp().gVersion); //version string need to come from settings
	info.SetDescription(wxT("Pharmacy mamagement system aid in the managment of pharmaceutical products, sale, transactions, prescription, expiry and so much more"));
	info.SetCopyright(wxT("(C) 2024 D-Glopa Technologies"));
	info.AddDeveloper("Ferife Zino :)");
	wxAboutBox(info);
}

void ab::MainFrame::OnIdle(wxIdleEvent& evt)
{
	int sel = mPager->GetSelection();
	if (sel == wxNOT_FOUND) return;

	switch (sel)
	{
	case WELCOME:
	{
		auto today = std::chrono::system_clock::now();
	

		std::chrono::sys_days dt = std::chrono::time_point_cast<std::chrono::sys_days::duration>(today);
		std::string todayTime  = fmt::format("{:%h:%m}"s, today);
		std::chrono::year_month_day ymd{ dt };
		std::chrono::weekday wk{ dt };


		std::stringstream os;
		os << fmt::format("{:%d} {}, {} {:%y}", dt, dayNames[wk.c_encoding()],
			monthNames[(std::uint32_t)ymd.month() - 1], dt);

		time1->Freeze();
		time1->SetLabelText(todayTime);
		time1->Thaw();
		time1->Refresh();

		date1->Freeze();
		date1->SetLabelText(os.str());
		date1->Thaw();
		date1->Refresh();

		mWelcomePage->Layout();
	}
	default:
		break;
	}
}

void ab::MainFrame::OnModuleActivated(const ab::mod& mod, ab::module_evt evt)
{
	if (evt == ab::module_evt::activated) {
		if (mod.win != nullptr) {
			mWorkspace->AddSpace(mod.win,
				mod.name, mod.img);
		}
	}
	if (mPager->GetSelection() == WELCOME)
		mPager->SetSelection(WORKSPACE);
}

void ab::MainFrame::OnWorkspaceNotif(ab::Workspace::notif notif, size_t page)
{
	switch (notif) {
	case ab::Workspace::notif::closed:
		if (mWorkspace->GetPageCount() == 0) {
			mPager->SetSelection(WELCOME);
		}
		break;
	}
}

void ab::MainFrame::OnAuiThemeChange()
{
	auto auiArt = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiArt);
	mManager.Update();
}
