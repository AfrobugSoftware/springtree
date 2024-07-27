#include "Authentication.hpp"

ab::Register::Register(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
 : wxDialog(parent, id, title, pos, size, style) {
}

ab::Register::~Register()
{
}

bool ab::Register::TransferDataFromWindow()
{
	return false;
}

ab::SignIn::SignIn(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
{
}

ab::SignIn::~SignIn()
{
}

bool ab::SignIn::TransferDataFromWindow()
{
	return false;
}
