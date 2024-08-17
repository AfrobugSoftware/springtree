#pragma once
#pragma once
#include <wx/aui/aui.h>
#include <wx/aui/tabart.h>
#include <wx/aui/dockart.h>
#include <wx/font.h>

#include <memory>
#include <boost/signals2.hpp>


namespace ab {
	//Monostate class to provide a theme for the Aui managed frames
	//also add dataviewctrl theme here
	class AuiTheme {
	public:
		using Signal_t = boost::signals2::signal<void(void)>;
		constexpr static unsigned long AUIMGRSTYLE = wxAUI_MGR_DEFAULT | wxAUI_MGR_TRANSPARENT_DRAG | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_LIVE_RESIZE;

		static Signal_t sSignal;
		static wxFont sFont;
		static wxFont sCaptionFont;

		static size_t sCaptionSize;
		static size_t sGripperSize;
		static size_t sSashSize;
		static size_t sPaneBorderSize;
		static size_t sGradientType;

		static wxColour sSashColour;
		static wxColour sBackgroundColour;
		static wxColour sBorderColour;
		static wxColour sActiveCaptionColour;
		static wxColour sActiveGradientColour;
		static wxColour sActiveTextCaptionColour;
		static wxColour sInactiveTextCaptionColour;
		static wxColour sInactiveCaptionColour;
		static wxColour sInactiveGradientColour;


		//theme upates
		static boost::signals2::connection Register(Signal_t::slot_function_type&& sig);
		static void Update(wxAuiDockArt* art);
		static void DarkTheme();
		static void LightTheme();
		static void DefaultTheme();

		AuiTheme() {};
	};
};