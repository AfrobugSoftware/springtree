#pragma once
#include <wx/aui/framemanager.h>
#include <wx/popupwin.h>
#include <wx/activityindicator.h>
#include <wx/stattext.h>
#include <wx/panel.h>


namespace ab {
	class WaitPopup : public wxPopupTransientWindow
	{
	public:
		WaitPopup(wxWindow* parent);
		virtual ~WaitPopup();
		void SetSize(const wxSize& size);


		wxAuiManager manager;
		wxPanel* mPanel = nullptr;
		wxActivityIndicator* mActivity = nullptr;
		wxStaticText* mText = nullptr;

	};
};