#pragma once
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/hyperlink.h>
#include <wx/statbmp.h>

namespace ab {
	class Register : public wxDialog {
	public:
		wxPanel* MainPane = nullptr;
		wxPanel* CreateAccount = nullptr;
		wxScrolledWindow* m_scrolledWindow1 = nullptr;
		wxPanel* m_panel5 = nullptr;
		wxStaticText* m_staticText12 = nullptr;
		wxStaticText* mLastNameLabel = nullptr;
		wxTextCtrl* mLastNameValue = nullptr;
		wxStaticText* mFirstNameLabel = nullptr;
		wxTextCtrl* mFirstNameValue = nullptr;
		wxStaticText* mUserNameLabel = nullptr;
		wxTextCtrl* mUserNameValue = nullptr;
		wxStaticText* mAccountTypeLabel = nullptr;
		wxChoice* mAccountType = nullptr;
		wxChoice* mSecurityQuestions = nullptr;
		wxTextCtrl* mSecurityAnswer = nullptr;
		wxStaticText* mEmailLabel = nullptr;
		wxTextCtrl* mEmailValue = nullptr;
		wxStaticText* mPhoneNo = nullptr;
		wxTextCtrl* mPhoneNoValue = nullptr;
		wxStaticText* mPasswordLabel = nullptr;
		wxTextCtrl* mPasswordValue = nullptr;
		wxStaticText* mConfirmPasswordLabel = nullptr;
		wxTextCtrl* mConfirmPasswordValue = nullptr;
		wxCheckBox* m_checkBox1 = nullptr;
		wxStaticText* mRegNumberLabel = nullptr;
		wxTextCtrl* mRegNumValue = nullptr;
		wxRadioBox* m_radioBox2 = nullptr;
		wxStdDialogButtonSizer* m_sdbSizer3 = nullptr;
		wxButton* m_sdbSizer3Save = nullptr;
		wxButton* m_sdbSizer3Cancel = nullptr;
		wxFlexGridSizer* fgSizer1 = nullptr;


		Register(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Registration"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(848, 584), long style = wxDEFAULT_DIALOG_STYLE);
		virtual ~Register();
		virtual bool TransferDataFromWindow() override;
	};


	class SignIn : public wxDialog {
		wxPanel* RootPane = nullptr;
		wxPanel* m_panel2 = nullptr;
		wxPanel* m_panel3 = nullptr;
		wxStaticBitmap* m_bitmap1 = nullptr;
		wxStaticText* mWelcomText = nullptr;
		wxTextCtrl* mUserName = nullptr;
		wxTextCtrl* mPassword = nullptr;
		wxCheckBox* mKeepMeSigned = nullptr;
		wxPanel* m_panel5 = nullptr;
		wxButton* mLogOn = nullptr;
		wxButton* mSignup = nullptr;
		wxPanel* mPharmacySignupPanel = nullptr;
		wxHyperlinkCtrl* mForgotPasswordLink = nullptr;
		wxHyperlinkCtrl* mHelpLink = nullptr;
		wxPanel* m_panel4 = nullptr;
	public:
		enum {
			ID_LOGON,
			ID_SIGNUP,
			ID_FORGOT_PASS,
			ID_HELP,
		};

		SignIn(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(848, 584), long style = wxDEFAULT_DIALOG_STYLE);
		virtual ~SignIn();

		virtual bool TransferDataFromWindow() override;
	};
};
