#pragma once
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/scrolwin.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/valtext.h>
#include <wx/valnum.h>
#include <wx/textdlg.h>
#include <wx/combobox.h>

#include <optional>
#include <boost/algorithm/string.hpp>



namespace ab
{
	class AddProductDialog : public wxDialog
	{
	private:

	protected:
		wxPanel* m_panel1;
		wxStaticText* TitleText;
		wxScrolledWindow* m_panel2;
		wxPanel* m_panel4;
		wxPanel* mProductDetailsPanel;
		wxStaticText* mProductName;
		wxTextCtrl* mProductNameValue;
		wxStaticText* mProductGenericName;
		wxTextCtrl* mGenericNameValue;
		wxStaticText* mFormulation;
		wxChoice* mFormulationValue;
		wxStaticText* mStrength;
		wxTextCtrl* mStrengthValue;
		wxStaticText* mStrengthType;
		wxChoice* mStrengthTypeValue;
		wxStaticText* mClassLabel;
		wxChoice* mClassValue;
		wxPanel* m_panel5;
		wxPanel* mProductAddDetails;
		wxPanel* m_panel71;
		wxStaticText* mCostPriceLabel;
		wxTextCtrl* mCostPriceValue;
		wxStaticText* mSalePriceLabel;
		wxTextCtrl* mSalePriceValue;
		wxButton* mDoMarkup;
		wxStaticText* mCategoryLabel;
		wxChoice* mCategoryValue;
		wxStaticText* mPackageSizeLabel;
		wxTextCtrl* mPackageSizeValue;
		wxStaticText* mProductDescription;
		wxTextCtrl* mProductDescriptionValue;
		wxBitmapButton* mMoreDescription;
		wxStaticText* mDirectionForUse;
		wxTextCtrl* mDirForUseValue;
		wxBitmapButton* mMoreDirForUse;
		wxStaticText* mHealthConditions;
		wxTextCtrl* mHealthConditionsValue;
		wxBitmapButton* mMoreHealthConditons;
		wxStaticText* mSideEffectsLabel;
		wxTextCtrl* mSideEffectsValue;
		wxBitmapButton* mMoreSideffects;
		wxPanel* m_panel8;
		wxButton* mScanProduct;
		wxPanel* m_panel6;
		wxPanel* mProductInvenPanel;
		wxCheckBox* mAddInventory;
		wxCheckBox* mAddSupplier;
		wxStaticText* mBacthNumber;
		wxTextCtrl* mBatchNumbeValue;
		wxStaticText* m_staticText8;
		wxDatePickerCtrl* m_datePicker1;
		wxStaticText* mQuntity;
		wxTextCtrl* mQunatityValue;
		wxStaticText* mSupplierName;
		wxComboBox* mSuplierNameValue;
		wxStaticText* mCostPerUnitName;
		wxTextCtrl* mCostPerUnitValue;
		wxStaticText* mBarcodeValue;
		wxStaticText* mInvoiceText;
		wxTextCtrl* mInvoiceValue;

		wxPanel* m_panel7;
		wxStdDialogButtonSizer* m_sdbSizer2;
		wxButton* m_sdbSizer2OK;
		wxButton* m_sdbSizer2Cancel;


		wxArrayString ProductClassChoices;
		wxArrayString FormulationChoices;
		wxArrayString ExpChoices;
		wxArrayString StrengthChoices;
		double mFloatValidator = 0.0;
		std::string mScanProductString;
	public:
		//controls id
		enum {
			ID_PRODUCT_NAME = wxID_HIGHEST + 200,
			ID_SCAN_PRODUCT,
			ID_MORE_DESCRIP,
			ID_MORE_DIRFORUSE,
			ID_MORE_SIDEEFFECTS,
			ID_MORE_HEALTHCON,
			ID_INVENTORY_ADD,
			ID_MARKUP_COST,
		};
		AddProductDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("ADD PRODUCT"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(948, 584), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
		virtual ~AddProductDialog() {}

		virtual bool TransferDataFromWindow() override final;
		void OnClose(wxCloseEvent& evt);
		void OnScanProduct(wxCommandEvent& evt);
		void OnInventoryCheck(wxCommandEvent& evt);
		void OnMarkupCost(wxCommandEvent& evt);

		wxArrayString SetupSupplierName();

		DECLARE_EVENT_TABLE()
	};


};
