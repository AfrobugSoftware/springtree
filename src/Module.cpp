#include "Module.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::Modules, wxPanel)
	EVT_TREE_ITEM_ACTIVATED(ab::Modules::ID_TREE, ab::Modules::OnTreeActivated)
	EVT_TREE_ITEM_RIGHT_CLICK(ab::Modules::ID_TREE, ab::Modules::OnRightClick)
	EVT_TREE_BEGIN_DRAG(ab::Modules::ID_TREE, ab::Modules::OnDragBegin)
	EVT_TREE_END_DRAG(ab::Modules::ID_TREE, ab::Modules::OnDragEnd)


END_EVENT_TABLE()

ab::Modules::Modules(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style){
	SetDoubleBuffered(true);
	auto& app = wxGetApp();

	auto& account = app.mPharmacyManager.account;
	auto& pharmacy = app.mPharmacyManager.pharmacy;

	auto cap = [&](const std::string& name) -> std::string {
		if (name.empty()) return ""s;
		std::string ret = name;
		*ret.begin() = std::toupper(*ret.begin());
		return ret;
	};

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	bSizer2->Add(0, 20, 0, wxEXPAND, FromDIP(5));

	m_bitmap1 = new wxStaticBitmap(m_panel1, wxID_ANY, wxArtProvider::GetBitmap("pharmacist"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(m_bitmap1, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

	std::string AccountName = fmt::format("{} {}", cap(account.last_name), cap(account.first_name));
	m_staticText1 = new wxStaticText(m_panel1, wxID_ANY, AccountName, wxDefaultPosition, wxDefaultSize, 0);
	m_staticText1->Wrap(-1);
	bSizer2->Add(m_staticText1, 0, wxALIGN_CENTER | wxALL, FromDIP(1));

	//add the account type
	std::string AccountType = cap(app.mPharmacyManager.GetAccountTypeAsString());
	m_staticText3 = new wxStaticText(m_panel1, wxID_ANY, AccountType, wxDefaultPosition, wxDefaultSize, 0);
	m_staticText3->Wrap(-1);
	bSizer2->Add(m_staticText3, 0, wxALIGN_CENTER | wxALL, FromDIP(1));

	std::string PharmacyName = cap(pharmacy.name);

	m_staticText2 = new wxStaticText(m_panel1, wxID_ANY, PharmacyName, wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	//m_staticText2->SetFont(wxFont(wxFontInfo(12)));
	bSizer2->Add(m_staticText2, 0, wxALIGN_CENTER | wxALL, FromDIP(1));

	std::string pharmacyType = cap(app.mPharmacyManager.GetPharmacyTypeAsString());
	m_staticText4 = new wxStaticText(m_panel1, wxID_ANY, pharmacyType, wxDefaultPosition, wxDefaultSize, 0);
	m_staticText4->SetFont(wxFontInfo(6).AntiAliased());
	m_staticText4->Wrap(-1);
	bSizer2->Add(m_staticText4, 0, wxALIGN_CENTER | wxALL, FromDIP(1));

	bSizer2->Add(0, 5, 0, wxEXPAND, FromDIP(5));


	m_panel1->SetSizer(bSizer2);
	m_panel1->Layout();
	bSizer2->Fit(m_panel1);
	bSizer1->Add(m_panel1, 0, wxEXPAND | wxALL, FromDIP(0));
	m_panel1->SetBackgroundColour(wxTheColourDatabase->Find("module"));

	wxPanel* m_panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxSizer* bLineSizer = new wxBoxSizer(wxHORIZONTAL);
	bLineSizer->AddSpacer(FromDIP(20));
	auto line = new wxStaticLine(m_panel3, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	line->SetBackgroundColour(wxTheColourDatabase->Find("module"));

	bLineSizer->Add(line, wxSizerFlags().Proportion(1).Expand());
	bLineSizer->AddSpacer(FromDIP(20));

	m_panel3->SetSizer(bLineSizer);
	m_panel3->Layout();
	bLineSizer->Fit(m_panel3);
	m_panel3->SetBackgroundColour(wxTheColourDatabase->Find("module"));

	m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxVERTICAL);

	mModuleTree = new wxTreeCtrl(m_panel2, ID_TREE, wxDefaultPosition, wxDefaultSize, wxTR_EDIT_LABELS | wxTR_FULL_ROW_HIGHLIGHT | wxTR_NO_LINES | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT | wxTR_SINGLE | wxNO_BORDER | wxTR_HAS_VARIABLE_ROW_HEIGHT);
	bSizer3->AddSpacer(FromDIP(10));
	bSizer3->Add(mModuleTree, wxSizerFlags().Expand().Proportion(1).CenterHorizontal().Border(wxALL,FromDIP(10)));
	mModuleTree->SetDoubleBuffered(true);
	mModuleTree->SetBackgroundColour(wxTheColourDatabase->Find("module"));

	CreateTree();

	m_panel2->SetSizer(bSizer3);
	m_panel2->Layout();
	bSizer3->Fit(m_panel2);

	//bSizer1->AddSpacer(FromDIP(10));
	bSizer1->Add(m_panel3, 0, wxEXPAND, 0);
	//bSizer1->AddSpacer(FromDIP(10));

	bSizer1->Add(m_panel2, 1, wxEXPAND | wxALL, FromDIP(0));
	m_panel2->SetBackgroundColour(wxTheColourDatabase->Find("module"));

	this->SetSizer(bSizer1);
	this->Layout();
}

void ab::Modules::CreateTree()
{
	mModuleTree->SetIndent(20);
	wxTreeItemId root = mModuleTree->AddRoot("Root", -1);


	mPharmacy = mModuleTree->AppendItem(root, "Pharamacy", 4);
	mTransactions = mModuleTree->AppendItem(root, "Transactions", 5);


	mProducts = mModuleTree->AppendItem(mPharmacy, "Products", 0);
	mPaitents = mModuleTree->AppendItem(mPharmacy, "Patients", 2);
	mPrescriptions = mModuleTree->AppendItem(mPharmacy, "Prescriptions", 6);
	mPoisionBook = mModuleTree->AppendItem(mPharmacy, "Poision Book", 7);

	mSales = mModuleTree->AppendItem(mTransactions, "Sales", 1);
	mAuditTrails = mModuleTree->AppendItem(mTransactions, "Audit Trails", 8);
	mRequisitions = mModuleTree->AppendItem(mTransactions, "Requisitions", 10);
	mOrders = mModuleTree->AppendItem(mTransactions, "ADR Reports", 9);


	mModuleTree->Expand(mPharmacy);
	mModuleTree->Expand(mTransactions);
}

void ab::Modules::OnTreeActivated(wxTreeEvent& evt)
{
	auto id = evt.GetItem();
	if (!id.IsOk()) return;

	ab::mod* found = NULL;
	bool s = Search(mMods, id, &found);
	if (!s) return;
	found->callback(*found, module_evt::activated);
}

void ab::Modules::OnRightClick(wxTreeEvent& evt)
{
	auto id = evt.GetItem();
	if (!id.IsOk()) return;

	ab::mod* found = NULL;
	bool s = Search(mMods, id, &found);
	if (!s) return;
	found->callback(*found, module_evt::right_click);
}

void ab::Modules::OnDragBegin(wxTreeEvent& evt)
{
	auto item = evt.GetItem();
	if (!item.IsOk()) return;
	evt.Allow();
	auto iter = std::ranges::find_if(mMods,
		[&](const auto& p) {
			return (p.id == item);
	});
	ab::treeDnd dnd;
	ab::DataObject obj;
	ab::mod* found = nullptr;
	if (iter != mMods.end()) {
		found = &(*iter);
	}
	else {
		bool s = Search(mMods, item, &found);
		if (!s) return;
	}

	dnd.id = reinterpret_cast<size_t>(item.GetID());
	dnd.win = found->win;
	dnd.name = found->name;
	dnd.img = found->img;

	ab::DataObject::data_t out(grape::serial::get_size(dnd), 0x00);
	grape::serial::write(boost::asio::buffer(out), dnd);
	obj.SetData(std::move(out));
	wxDropSource source(obj);
	source.DoDragDrop(wxDrag_CopyOnly);
	evt.Veto();
}

void ab::Modules::OnDragEnd(wxTreeEvent& evt)
{
	evt.Veto();
}

void ab::Modules::OnBeginEditLabel(wxTreeEvent& evt)
{
	auto item = evt.GetItem();
	if (!item.IsOk()) {
		evt.Veto();
		return;
	}

	auto iter = std::ranges::find_if(mMods, [&](const auto& mod) {
		return (mod.id == item);
	});

	if (iter == mMods.end()) {
		evt.Veto();
		return;
	}

	evt.Skip();
}

void ab::Modules::OnEndEditLabel(wxTreeEvent& evt)
{
	auto id = evt.GetItem();
	if (!id.IsOk()) return;

	std::string oldname = mModuleTree->GetItemText(id).ToStdString();
	auto name = evt.GetLabel().ToStdString();
	if (name.empty() || oldname == name) {
		evt.Veto();
		return;
	}

	mLabelChange(id, oldname, name);
	evt.Skip(); //why ?
}

void ab::Modules::ReloadAccountDetails()
{
	auto& app = wxGetApp();
	auto& account = app.mPharmacyManager.account;
	auto& pharmacy = app.mPharmacyManager.pharmacy;

	auto cap = [&](const std::string& name) -> std::string {
		if(name.empty()) return ""s;
		std::string ret = name;
		*ret.begin() = std::toupper(*ret.begin());
		return ret;
	};


	std::string AccountType = app.mPharmacyManager.GetAccountTypeAsString();
	
	std::string AccountName = fmt::format("{} {}", cap(account.last_name), cap(account.first_name));
	std::string PharmacyName = cap(pharmacy.name);
	std::string PharmacyType = cap(app.mPharmacyManager.GetPharmacyTypeAsString());

	m_panel1->Freeze();

	m_staticText1->SetLabel(std::move(AccountName));
	m_staticText2->SetLabel(std::move(PharmacyName));
	m_staticText3->SetLabel(std::move(AccountType));
	m_staticText4->SetLabel(std::move(PharmacyType));

	m_panel1->Layout();
	m_panel1->Thaw();
	m_panel1->Refresh();
}

void ab::Modules::UpdatesLogo(const wxBitmap& bm)
{
	if (!bm.IsOk()) return;
	//check dimentions ?
	//think about scaling the bitmap to fit

	m_panel1->Freeze();
	m_bitmap1->SetBitmap(bm);
	m_panel1->Layout();
	m_panel1->Thaw();
}

void ab::Modules::ActivateModule(const wxTreeItemId& id)
{
	ab::mod* found = NULL;
	bool s = Search(mMods, id, &found);
	if (!s) return;
	found->callback(*found, module_evt::activated);
}

void ab::Modules::Add(ab::mod&& mod)
{
	//please do not add duplicates
	mMods.push_back(std::forward<ab::mod>(mod));
}

wxTreeItemId ab::Modules::AddChild(const wxTreeItemId& parent, ab::mod&& mod)
{
	auto iter = std::ranges::find_if(mMods,
		[&](const auto& p) {
			return (p.id == parent);
	});
	if (iter == mMods.end()) {
		ab::mod* found = nullptr;
		bool s = Search(mMods, parent, &found);
		if (!s) return {};

		auto item = mModuleTree->AppendItem(parent, mod.name, mod.img);
		mod.id = item;
		found->children.push_back(std::forward<ab::mod>(mod));
		return item;
	}
	else {
		auto item = mModuleTree->AppendItem(parent, mod.name, mod.img);
		mod.id = item;
		iter->children.push_back(std::forward<ab::mod>(mod));
		return item;
	}
}

bool ab::Modules::RemoveChild(const wxTreeItemId& parent, const ab::mod& mod)
{
	auto iter = std::ranges::find_if(mMods,
		[&](const auto& p) {
			return (p.id == parent);
		});
	if (iter != mMods.end()) {
		mModuleTree->Delete(mod.id);
		auto& l = iter->children;
		auto r = std::remove_if(l.begin(), l.end(), [&](const auto& ii) -> bool {
			return (ii.id == mod.id);
		});
		l.erase(r, std::end(l));
		return true;
	}
	else {
		ab::mod* found = nullptr;
		bool s = Search(mMods, parent, &found);
		if (!s) return false;

		auto& l = found->children;
		auto r = std::remove_if(l.begin(), l.end(), [&](const auto& ii) -> bool {
			return (ii.id == mod.id);
			});
		l.erase(r, std::end(l));
		return true;
	}
}

bool ab::Modules::Search(std::vector<ab::mod>& hey, const wxTreeItemId& needle, ab::mod** found)
{
	if (hey.empty()) return false;
	for (auto& m : hey) {
		if (m.id == needle) {
			*found = &m;
			return true;
		}
		if (Search(m.children, needle, found)) return true;
	}
	return false;
}

//bool ab::operator==(const wxTreeItemId& a, const wxTreeItemId& b)
//{
//	return (a == b);
//}

std::size_t ab::hash_value(wxTreeItemId const& b)
{
	const size_t i = reinterpret_cast<size_t>(b.GetID());
	return boost::hash_value(i);
}
