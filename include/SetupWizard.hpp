#pragma once
#include "wx/frame.h"
#include "wx/stattext.h"
#include "wx/log.h"
#include "wx/app.h"
#include "wx/checkbox.h"
#include "wx/checklst.h"
#include "wx/msgdlg.h"
#include "wx/radiobox.h"
#include "wx/menu.h"
#include "wx/sizer.h"

#include "wx/textctrl.h"
#include "wx/wizard.h"
#include <wx/config.h>
#include <wx/artprov.h>
#include <wx/choice.h>
#include <wx/dcclient.h>
#include <wx/button.h>

#include "Application.hpp"


#include <boost/algorithm/string.hpp>

namespace ab {
	class SetupWizard : public wxWizard {
		wxPanel* m_panel1;
		wxStaticText* mTitle;
		wxStaticText* mDescription;
		wxScrolledWindow* m_scrolledWindow1;
		wxPanel* m_panel3;
		wxStaticText* mPharmacyType;
		wxChoice* mPharmacyTypeValue;
		wxStaticText* mPharamcyName;
		wxTextCtrl* mPharmacyNameValue;
		wxStaticText* mBranchName;
		wxTextCtrl* mBranchNameValue;
		wxStaticText* mPhoneNo;
		wxTextCtrl* mPhoneNoValue;
		wxStaticText* mEmail;
		wxTextCtrl* mEmailValue;
		wxStaticText* mWebsiteText;
		wxTextCtrl* mWebsiteValue;
		wxPanel* m_panel4;
		wxStaticText* mCountyText;
		wxTextCtrl* mCountryValue;
		wxStaticText* mLgaText;
		wxTextCtrl* mLgaValue;
		wxStaticText* mNoText;
		wxTextCtrl* mNoValue;
		wxStaticText* mStreetText;
		wxTextCtrl* mStreetValue;
		wxStaticText* mCityText;
		wxTextCtrl* mCityValue;
		wxStaticText* mStateText;
		wxTextCtrl* mStateValue;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	public:
		enum {
			ID_FIRST_PAGE = wxID_HIGHEST + 100,
			ID_ADD_ACCOUNT,
		};

		SetupWizard(wxFrame* frame);
		virtual ~SetupWizard() {}

		grape::pharmacy pharmacy;
		grape::account account;
		grape::branch branch;
		grape::address address;


		constexpr bool GetState() const { return state; }
	private:
		void OnFinished(wxWizardEvent& evt);
		void OnAddAccount(wxCommandEvent& evt);

		virtual bool TransferDataFromWindow() override;

		//creation functions
		void CreateFirstPage();
		void CreateContactPage();
		void CreateAddressPage();
		void CreateBranchPage();
		void CreateAddAccountPage();
		void CreateSummaryPage();


		wxWizardPageSimple* mFirstPage = nullptr;
		wxWizardPageSimple* mContactPage = nullptr;
		wxWizardPageSimple* mAddressPage = nullptr;
		wxWizardPageSimple* mBranchPage = nullptr;
		wxWizardPageSimple* mAddAccountPage = nullptr;
		wxWizardPageSimple* mSummaryPage = nullptr;
		wxButton* btn = nullptr;
		
		wxSize pageSize;
		bool state = false;
		DECLARE_EVENT_TABLE()

	};
};