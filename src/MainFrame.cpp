#include "MainFrame.hpp"

ab::MainFrame::MainFrame()
{
}

ab::MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size)
 : wxFrame(parent, id, "PharmaOffice - enterprise", position, size) {
	SetSize(FromDIP(size));
}

ab::MainFrame::~MainFrame()
{
}
