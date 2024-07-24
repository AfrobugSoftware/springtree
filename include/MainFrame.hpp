#pragma once
#include <wx/frame.h>

namespace ab {
	class MainFrame : public wxFrame
	{
	public:
		MainFrame();
		MainFrame(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size);
		virtual ~MainFrame();

	};
};