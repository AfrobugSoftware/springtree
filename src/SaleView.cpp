#include "SaleView.hpp"
BEGIN_EVENT_TABLE(ab::SaleView, wxPanel)
	EVT_BUTTON(ab::SaleView::ID_CHECKOUT, ab::SaleView::OnCheckOut)
END_EVENT_TABLE()


ab::SaleView::SaleView(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	: wxPanel(parent, id, position, size, style) {
	SetupAuiTheme();
}

ab::SaleView::~SaleView()
{
}

void ab::SaleView::SetupAuiTheme()
{
	auto auiart = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
	ab::AuiTheme::Register(std::bind_front(&ab::SaleView::OnAuiThemeChange, this));
}

void ab::SaleView::OnAuiThemeChange()
{
	auto auiart = mManager.GetArtProvider();
	ab::AuiTheme::Update(auiart);
}

void ab::SaleView::OnCheckOut(wxCommandEvent& evt)
{
}
