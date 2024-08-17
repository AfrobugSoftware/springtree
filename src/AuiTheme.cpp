#include "AuiTheme.hpp"
boost::signals2::signal<void(void)> ab::AuiTheme::sSignal;
wxFont ab::AuiTheme::sFont;
wxFont ab::AuiTheme::sCaptionFont;

size_t ab::AuiTheme::sCaptionSize = 25;
size_t ab::AuiTheme::sGripperSize = 5;
size_t ab::AuiTheme::sSashSize = 5;
size_t ab::AuiTheme::sPaneBorderSize = 0;
size_t ab::AuiTheme::sGradientType = wxAUI_GRADIENT_HORIZONTAL;

wxColour ab::AuiTheme::sSashColour = *wxWHITE;
wxColour ab::AuiTheme::sBackgroundColour = *wxWHITE;
wxColour ab::AuiTheme::sBorderColour = *wxBLACK;
wxColour ab::AuiTheme::sActiveCaptionColour = *wxLIGHT_GREY;
wxColour ab::AuiTheme::sActiveGradientColour = *wxWHITE;
wxColour ab::AuiTheme::sActiveTextCaptionColour = *wxBLACK;
wxColour ab::AuiTheme::sInactiveTextCaptionColour = *wxBLACK;
wxColour ab::AuiTheme::sInactiveCaptionColour = *wxLIGHT_GREY;
wxColour ab::AuiTheme::sInactiveGradientColour = *wxWHITE;

void ab::AuiTheme::Update(wxAuiDockArt* art)
{
	art->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, sCaptionSize);
	art->SetMetric(wxAUI_DOCKART_GRIPPER_SIZE, sGripperSize);
	art->SetMetric(wxAUI_DOCKART_SASH_SIZE, sSashSize);
	art->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, sPaneBorderSize);
	art->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, sGradientType);



	art->SetColour(wxAUI_DOCKART_SASH_COLOUR, sSashColour);
	art->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, sBackgroundColour);
	art->SetColour(wxAUI_DOCKART_BORDER_COLOUR, sBorderColour);
	art->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, sActiveCaptionColour);
	art->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, sActiveGradientColour);
	art->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR, sActiveTextCaptionColour);
	art->SetColor(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, sInactiveTextCaptionColour);
	art->SetColor(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, sInactiveCaptionColour);
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, sInactiveGradientColour);
}

void ab::AuiTheme::DarkTheme()
{

}

void ab::AuiTheme::LightTheme()
{
}

void ab::AuiTheme::DefaultTheme()
{
}

boost::signals2::connection ab::AuiTheme::Register(ab::AuiTheme::Signal_t::slot_function_type&& func)
{
	return sSignal.connect(std::forward<ab::AuiTheme::Signal_t::slot_function_type>(func));
}