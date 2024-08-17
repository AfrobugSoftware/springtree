#include "WaitPopup.hpp"

ab::WaitPopup::WaitPopup(wxWindow* parent)
: wxPopupTransientWindow(parent, wxBORDER_NONE), manager(this){

    mPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(300, 260)), wxTAB_TRAVERSAL | wxNO_BORDER);
   // mPanel->SetBackgroundColour(*wxBLUE);
    wxBoxSizer* sz = new wxBoxSizer(wxVERTICAL);

    mActivity = new wxActivityIndicator(mPanel, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(300, 300)));
    mText = new wxStaticText(mPanel, wxID_ANY, "C", wxDefaultPosition, FromDIP(wxSize(150, 150)));
    mText->Wrap(-1);

    sz->Add(mActivity, wxSizerFlags().CenterHorizontal().Border(wxALL, FromDIP(2)));
    sz->AddSpacer(FromDIP(5));
    sz->Add(mText, wxSizerFlags().CenterHorizontal().Border(wxALL, FromDIP(2)));

    mPanel->SetSizer(sz);
    mPanel->Layout();

    manager.AddPane(mPanel, wxAuiPaneInfo().Name("Pane").CenterPane().Show());
    manager.Update();
}

ab::WaitPopup::~WaitPopup()
{
	manager.UnInit();
}

void ab::WaitPopup::SetSize(const wxSize& size)
{
    SetClientSize(size);
    wxPoint pos = GetParent()->ClientToScreen(wxPoint{ 0,0 });
    wxSize psize = GetParent()->GetSize();

    wxPoint npos = wxPoint{ (pos.x + psize.x /2) - size.x / 2, (pos.y + psize.y /2) - size.y / 2 };
    SetPosition(npos);
}
