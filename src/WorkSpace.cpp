#include "WorkSpace.hpp"
BEGIN_EVENT_TABLE(ab::Workspace, wxPanel)
EVT_AUINOTEBOOK_PAGE_CLOSE(ab::Workspace::WORKSPACEBOOK, ab::Workspace::OnWorkspaceClose)
END_EVENT_TABLE()



ab::Workspace::Workspace(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
	wxPanel(parent, id, pos, size, style)
{
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);
	SetBackgroundColour(*wxWHITE);
	//SetDoubleBuffered(true);

	mWorkspacebook = new wxAuiNotebook(this, WORKSPACEBOOK, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TOP | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	mWorkspacebook->SetDoubleBuffered(true);


	bSizer4->Add(mWorkspacebook, 1, wxEXPAND | wxALL, 0);

	SetDropTarget(new ab::DropTarget(new ab::DataObject(), std::bind_front(&ab::Workspace::OnDroppedTreeItem, this)));

	this->SetSizer(bSizer4);
	this->Layout();
}

bool ab::Workspace::AddSpace(wxWindow* space, const std::string& name, int img)
{
	if (!space) return false;
	// check if already inserted
	auto pageidx = mWorkspacebook->GetPageIndex(space);
	if (pageidx != wxNOT_FOUND) {
		if (!space->IsShown()) space->Show();
		mWorkspacebook->SetSelection(pageidx);
		notifsignal(ab::Workspace::notif::added, pageidx);
		return true;
	}
	auto ret = mWorkspacebook->AddPage(space, name, true, img);
	notifsignal(ab::Workspace::notif::added, mWorkspacebook->GetSelection());
	return ret;
}


void ab::Workspace::OnWorkspaceClose(wxAuiNotebookEvent& evt)
{
	auto pageIndex = evt.GetSelection();
	if (pageIndex != wxNOT_FOUND) {
		mWorkspacebook->RemovePage(pageIndex);
		notifsignal(notif::closed, pageIndex);
	}
	evt.Veto();
}

void ab::Workspace::OnDroppedTreeItem(const ab::DataObject& item)
{
	auto opt = item.GetData<ab::treeDnd>();
	if (!opt.has_value()) return;
	auto& i = opt.value();
	auto win = static_cast<wxWindow*>(i.win);
	mDropTreeSignal(wxTreeItemId(reinterpret_cast<void*>(i.id)), i.name);
	if(win) AddSpace(win, i.name, i.img);
}