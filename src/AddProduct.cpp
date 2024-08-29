#include "AddProduct.hpp"
#include "Application.hpp"

BEGIN_EVENT_TABLE(ab::AddProductDialog, wxDialog)
	EVT_BUTTON(ab::AddProductDialog::ID_SCAN_PRODUCT, ab::AddProductDialog::OnScanProduct)
	EVT_CLOSE(ab::AddProductDialog::OnClose)
END_EVENT_TABLE()

ab::AddProductDialog::AddProductDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxDialog(parent, id, title, pos, size, style){
	this->SetSize(FromDIP(wxSize(948, 584)));
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));
	wxBusyCursor cusor;

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_panel1->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	TitleText = new wxStaticText(m_panel1, wxID_ANY, wxT("Add Product"), wxDefaultPosition, wxDefaultSize, 0);
	TitleText->Wrap(-1);
	TitleText->SetFont(wxFont(wxFontInfo(FromDIP(10)).AntiAliased().Bold()));

	bSizer2->Add(TitleText, 1, wxALL, FromDIP(5));

	wxStaticText* ext = new wxStaticText(m_panel1, wxID_ANY, wxT("Creates a product for the pharmacy"), wxDefaultPosition, wxDefaultSize, 0);
	ext->Wrap(-1);
	ext->SetFont(wxFont(wxFontInfo().AntiAliased()));

	bSizer2->Add(ext, 0, wxALL, 2);

	m_panel1->SetSizer(bSizer2);
	m_panel1->Layout();
	bSizer2->Fit(m_panel1);
	bSizer1->Add(m_panel1, 0, wxEXPAND | wxALL, 5);

	m_panel2 = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
	m_panel2->SetScrollRate(5, 5);
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxVERTICAL);

	m_panel4 = new wxPanel(m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer(new wxStaticBox(m_panel4, wxID_ANY, wxT("Product Details")), wxVERTICAL);

	mProductDetailsPanel = new wxPanel(sbSizer7->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer2->AddGrowableCol(1);
	fgSizer2->SetFlexibleDirection(wxBOTH);
	fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	mProductName = new wxStaticText(mProductDetailsPanel, wxID_ANY, wxT("Product Brand Name"), wxDefaultPosition, wxDefaultSize, 0);
	mProductName->Wrap(-1);
	fgSizer2->Add(mProductName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mProductNameValue = new wxTextCtrl(mProductDetailsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mProductNameValue->SetValidator(wxTextValidator{ wxFILTER_EMPTY });
	fgSizer2->Add(mProductNameValue, 1, wxALL | wxEXPAND, 5);

	mProductGenericName = new wxStaticText(mProductDetailsPanel, wxID_ANY, wxT("Generic Name"), wxDefaultPosition, wxDefaultSize, 0);
	mProductGenericName->Wrap(-1);
	fgSizer2->Add(mProductGenericName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mGenericNameValue = new wxTextCtrl(mProductDetailsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(mGenericNameValue, 1, wxALL | wxEXPAND, 5);

	mFormulation = new wxStaticText(mProductDetailsPanel, wxID_ANY, wxT("Formulation"), wxDefaultPosition, wxDefaultSize, 0);
	mFormulation->Wrap(-1);
	fgSizer2->Add(mFormulation, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	FormulationChoices.Add("TABLET");
	FormulationChoices.Add("CAPSULE");
	FormulationChoices.Add("SOLUTION");
	FormulationChoices.Add("SUSPENSION");
	FormulationChoices.Add("SYRUP");
	FormulationChoices.Add("IV");
	FormulationChoices.Add("IM");
	FormulationChoices.Add("EMULSION");
	FormulationChoices.Add("CREAM");
	FormulationChoices.Add("COMSUMABLE"); //needles, cannula and the rest
	FormulationChoices.Add("POWDER"); //needles, cannula and the rest
	FormulationChoices.Add("OINTMNET"); //needles, cannula and the rest
	FormulationChoices.Add("EYE DROP"); //needles, cannula and the rest
	FormulationChoices.Add("SUPPOSITORY"); //needles, cannula and the rest
	FormulationChoices.Add("LOZENGES"); //needles, cannula and the rest

	FormulationChoices.Add("NOT SPECIFIED"); //NOT SPECIFIED

	mFormulationValue = new wxChoice(mProductDetailsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, FormulationChoices, 0);
	mFormulationValue->SetSelection(0);
	mFormulationValue->SetBackgroundColour(wxColour(255, 255, 255));

	fgSizer2->Add(mFormulationValue, 0, wxALL | wxEXPAND, 5);

	mStrength = new wxStaticText(mProductDetailsPanel, wxID_ANY, wxT("Strength"), wxDefaultPosition, wxDefaultSize, 0);
	mStrength->Wrap(-1);
	fgSizer2->Add(mStrength, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxFloatingPointValidator<double> val(2, &mFloatValidator, wxNUM_VAL_ZERO_AS_BLANK);
	val.SetRange(0, 999999999999);

	mStrengthValue = new wxTextCtrl(mProductDetailsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer2->Add(mStrengthValue, 1, wxALL | wxEXPAND, 5);

	mStrengthType = new wxStaticText(mProductDetailsPanel, wxID_ANY, wxT("Strength Type"), wxDefaultPosition, wxDefaultSize, 0);
	mStrengthType->Wrap(-1);
	fgSizer2->Add(mStrengthType, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	StrengthChoices.Add("NOT SPECIFIED");
	StrengthChoices.Add("g");
	StrengthChoices.Add("mg");
	StrengthChoices.Add("mcg");

	StrengthChoices.Add("L");
	StrengthChoices.Add("ml");

	StrengthChoices.Add("%v/v");
	StrengthChoices.Add("%w/v");

	StrengthChoices.Add("mol");
	StrengthChoices.Add("mmol");
	mStrengthTypeValue = new wxChoice(mProductDetailsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, StrengthChoices, 0);
	mStrengthTypeValue->SetSelection(0);
	fgSizer2->Add(mStrengthTypeValue, 1, wxALL | wxEXPAND, 5);

	mClassLabel = new wxStaticText(mProductDetailsPanel, wxID_ANY, wxT("Product Class"), wxDefaultPosition, wxDefaultSize, 0);
	mClassLabel->Wrap(-1);
	fgSizer2->Add(mClassLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	ProductClassChoices.Add("POM");
	ProductClassChoices.Add("OTC");
	ProductClassChoices.Add("CONTROLLED");
	mClassValue = new wxChoice(mProductDetailsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, ProductClassChoices, 0);
	mClassValue->SetSelection(0);
	fgSizer2->Add(mClassValue, 1, wxALL | wxEXPAND, 5);


	mProductDetailsPanel->SetSizer(fgSizer2);
	mProductDetailsPanel->Layout();
	fgSizer2->Fit(mProductDetailsPanel);
	sbSizer7->Add(mProductDetailsPanel, 1, wxEXPAND | wxALL, 5);


	m_panel4->SetSizer(sbSizer7);
	m_panel4->Layout();
	sbSizer7->Fit(m_panel4);
	bSizer3->Add(m_panel4, 0, wxEXPAND | wxALL, 5);

	m_panel5 = new wxPanel(m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer(new wxStaticBox(m_panel5, wxID_ANY, wxT("Product Additional Details")), wxVERTICAL);

	mProductAddDetails = new wxPanel(sbSizer8->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxVERTICAL);

	m_panel71 = new wxPanel(mProductAddDetails, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer(wxHORIZONTAL);

	mCostPriceLabel = new wxStaticText(m_panel71, wxID_ANY, wxT("Cost Price"), wxDefaultPosition, wxDefaultSize, 0);
	mCostPriceLabel->Wrap(-1);
	bSizer7->Add(mCostPriceLabel, 0, wxALIGN_CENTER | wxALL, 5);

	mCostPriceValue = new wxTextCtrl(m_panel71, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mCostPriceValue->SetValidator(val);
	bSizer7->Add(mCostPriceValue, 0, wxALL | wxALIGN_CENTER, 5);

	mSalePriceLabel = new wxStaticText(m_panel71, wxID_ANY, wxT("Sale Price"), wxDefaultPosition, wxDefaultSize, 0);
	mSalePriceLabel->Wrap(-1);
	bSizer7->Add(mSalePriceLabel, 0, wxALIGN_CENTER | wxALL, 5);

	mSalePriceValue = new wxTextCtrl(m_panel71, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mSalePriceValue->SetValidator(val);
	bSizer7->Add(mSalePriceValue, 0, wxALL | wxALIGN_CENTER, 5);

	mDoMarkup = new wxButton(m_panel71, ID_MARKUP_COST, wxT("Mark up cost price"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer7->Add(mDoMarkup, 0, wxBOTTOM | wxLEFT | wxTOP | wxALIGN_CENTER, 10);

	mCategoryLabel = new wxStaticText(m_panel71, wxID_ANY, wxT("Category"), wxDefaultPosition, wxDefaultSize, 0);
	mCategoryLabel->Wrap(-1);
	bSizer7->Add(mCategoryLabel, 0, wxALIGN_CENTER | wxALL, 5);

	wxArrayString mCategoryValueChoices;
	mCategoryValueChoices.push_back("No Category");
	mCategoryValue = new wxChoice(m_panel71, wxID_ANY, wxDefaultPosition, wxDefaultSize, mCategoryValueChoices, 0);
	mCategoryValue->SetSelection(0);
	bSizer7->Add(mCategoryValue, 1, wxALL | wxALIGN_CENTER, 5);

	mPackageSizeLabel = new wxStaticText(m_panel71, wxID_ANY, wxT("Package Size"), wxDefaultPosition, wxDefaultSize, 0);
	mPackageSizeLabel->Wrap(-1);
	bSizer7->Add(mPackageSizeLabel, 0, wxALIGN_CENTER | wxALL, 5);

	mPackageSizeValue = new wxTextCtrl(m_panel71, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mPackageSizeValue->SetValidator(wxTextValidator{ wxFILTER_DIGITS });
	bSizer7->Add(mPackageSizeValue, 0, wxALL | wxALIGN_CENTER, 5);


	m_panel71->SetSizer(bSizer7);
	m_panel71->Layout();
	bSizer7->Fit(m_panel71);
	bSizer5->Add(m_panel71, 0, wxEXPAND | wxALL, 2);

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer3->AddGrowableCol(1);
	fgSizer3->SetFlexibleDirection(wxBOTH);
	fgSizer3->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	mProductDescription = new wxStaticText(mProductAddDetails, wxID_ANY, wxT("Description"), wxDefaultPosition, wxDefaultSize, 0);
	mProductDescription->Wrap(-1);
	fgSizer3->Add(mProductDescription, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mProductDescriptionValue = new wxTextCtrl(mProductAddDetails, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer3->Add(mProductDescriptionValue, 1, wxALL | wxEXPAND, 5);

	mDirectionForUse = new wxStaticText(mProductAddDetails, wxID_ANY, wxT("Direction For Use"), wxDefaultPosition, wxDefaultSize, 0);
	mDirectionForUse->Wrap(-1);
	fgSizer3->Add(mDirectionForUse, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mDirForUseValue = new wxTextCtrl(mProductAddDetails, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer3->Add(mDirForUseValue, 0, wxALL | wxEXPAND, 5);

	mHealthConditions = new wxStaticText(mProductAddDetails, wxID_ANY, wxT("Health Conditions"), wxDefaultPosition, wxDefaultSize, 0);
	mHealthConditions->Wrap(-1);
	fgSizer3->Add(mHealthConditions, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mHealthConditionsValue = new wxTextCtrl(mProductAddDetails, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer3->Add(mHealthConditionsValue, 0, wxALL | wxEXPAND, 5);


	mSideEffectsLabel = new wxStaticText(mProductAddDetails, wxID_ANY, wxT("Adverse Reactions"), wxDefaultPosition, wxDefaultSize, 0);
	mSideEffectsLabel->Wrap(-1);
	fgSizer3->Add(mSideEffectsLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mSideEffectsValue = new wxTextCtrl(mProductAddDetails, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer3->Add(mSideEffectsValue, 1, wxALL | wxEXPAND, 5);

	bSizer5->Add(fgSizer3, 1, wxEXPAND, 5);

	m_panel8 = new wxPanel(mProductAddDetails, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxHORIZONTAL);

	mScanProduct = new wxButton(m_panel8, ID_SCAN_PRODUCT, wxT("Scan Product"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer8->Add(mScanProduct, 0, wxALIGN_CENTER | wxALL, 0);


	m_panel8->SetSizer(bSizer8);
	m_panel8->Layout();
	bSizer8->Fit(m_panel8);
	bSizer5->Add(m_panel8, 0, wxEXPAND | wxALL, 5);


	mProductAddDetails->SetSizer(bSizer5);
	mProductAddDetails->Layout();
	bSizer5->Fit(mProductAddDetails);
	sbSizer8->Add(mProductAddDetails, 1, wxEXPAND | wxALL, 5);


	m_panel5->SetSizer(sbSizer8);
	m_panel5->Layout();
	sbSizer8->Fit(m_panel5);
	bSizer3->Add(m_panel5, 0, wxALL | wxEXPAND | wxFIXED_MINSIZE, 5);

	m_panel6 = new wxPanel(m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(m_panel6, wxID_ANY, wxT("Product Inventory")), wxVERTICAL);

	mProductInvenPanel = new wxPanel(sbSizer3->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFlexGridSizer* fgSizer21;
	fgSizer21 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer21->AddGrowableCol(1);
	fgSizer21->SetFlexibleDirection(wxBOTH);
	fgSizer21->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	mAddInventory = new wxCheckBox(mProductInvenPanel, ID_INVENTORY_ADD, wxT("Add inventory"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer21->Add(mAddInventory, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mAddSupplier = new wxCheckBox(mProductInvenPanel, ID_INVENTORY_ADD, wxT("Add supplier"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer21->Add(mAddSupplier, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	mBacthNumber = new wxStaticText(mProductInvenPanel, wxID_ANY, wxT("Batch Number"), wxDefaultPosition, wxDefaultSize, 0);
	mBacthNumber->Wrap(-1);
	fgSizer21->Add(mBacthNumber, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mBatchNumbeValue = new wxTextCtrl(mProductInvenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mBatchNumbeValue->SetValidator(wxTextValidator{ wxFILTER_DIGITS });
	fgSizer21->Add(mBatchNumbeValue, 1, wxALL | wxEXPAND, 5);

	m_staticText8 = new wxStaticText(mProductInvenPanel, wxID_ANY, wxT("Product Expiry Date"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText8->Wrap(-1);
	fgSizer21->Add(m_staticText8, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	m_datePicker1 = new wxDatePickerCtrl(mProductInvenPanel, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT | wxDP_DROPDOWN);
	m_datePicker1->SetRange(wxDateTime::Now(), wxDateTime{});
	fgSizer21->Add(m_datePicker1, 1, wxALL | wxEXPAND, 5);

	mQuntity = new wxStaticText(mProductInvenPanel, wxID_ANY, wxT("Quantity"), wxDefaultPosition, wxDefaultSize, 0);
	mQuntity->Wrap(-1);
	fgSizer21->Add(mQuntity, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mQunatityValue = new wxTextCtrl(mProductInvenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mQunatityValue->SetValidator(wxTextValidator{ wxFILTER_DIGITS });
	fgSizer21->Add(mQunatityValue, 0, wxALL | wxEXPAND, 5);

	mSupplierName = new wxStaticText(mProductInvenPanel, wxID_ANY, wxT("Supplier Name"), wxDefaultPosition, wxDefaultSize, 0);
	mSupplierName->Wrap(-1);
	fgSizer21->Add(mSupplierName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	auto suppliers = SetupSupplierName();
	mSuplierNameValue = new wxComboBox(mProductInvenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, suppliers, 0);
	fgSizer21->Add(mSuplierNameValue, 0, wxALL | wxEXPAND, 5);

	mInvoiceText = new wxStaticText(mProductInvenPanel, wxID_ANY, wxT("Invoice"), wxDefaultPosition, wxDefaultSize, 0);
	mInvoiceText->Wrap(-1);
	fgSizer21->Add(mInvoiceText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mInvoiceValue = new wxTextCtrl(mProductInvenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	fgSizer21->Add(mInvoiceValue, 0, wxALL | wxEXPAND, 5);


	mCostPerUnitName = new wxStaticText(mProductInvenPanel, wxID_ANY, wxT("Cost Price Per Unit(N)"), wxDefaultPosition, wxDefaultSize, 0);
	mSupplierName->Wrap(-1);
	fgSizer21->Add(mCostPerUnitName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	mCostPerUnitValue = new wxTextCtrl(mProductInvenPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	mCostPerUnitValue->SetValidator(val);
	fgSizer21->Add(mCostPerUnitValue, 0, wxALL | wxEXPAND, 5);


	mProductInvenPanel->SetSizer(fgSizer21);
	mProductInvenPanel->Layout();
	fgSizer21->Fit(mProductInvenPanel);
	sbSizer3->Add(mProductInvenPanel, 1, wxEXPAND | wxALL, 5);


	m_panel6->SetSizer(sbSizer3);
	m_panel6->Layout();
	sbSizer3->Fit(m_panel6);
	bSizer3->Add(m_panel6, 1, wxEXPAND | wxALL, 5);


	m_panel2->SetSizer(bSizer3);
	m_panel2->Layout();
	bSizer3->Fit(m_panel2);
	bSizer1->Add(m_panel2, 1, wxEXPAND | wxALL, 5);

	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton(this, wxID_OK);
	m_sdbSizer2->AddButton(m_sdbSizer2OK);
	m_sdbSizer2Cancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizer2->AddButton(m_sdbSizer2Cancel);
	m_sdbSizer2->Realize();


	bSizer1->Add(m_sdbSizer2, 0, wxEXPAND | wxALL, 5);

	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);
	SetIcon(wxGetApp().mAppIcon);
}

bool ab::AddProductDialog::TransferDataFromWindow()
{
	try {
		auto& app = wxGetApp();
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
		grape::credentials cred;
		cred.pharm_id   = app.mPharmacyManager.pharmacy.id;
		cred.branch_id  = app.mPharmacyManager.branch.id;
		cred.account_id = app.mPharmacyManager.account.account_id;
		cred.session_id = app.mPharmacyManager.account.session_id.value();

		constexpr size_t cred_size = grape::serial::get_size(cred);
		grape::session::response_type resp{};
		grape::body_type body(cred_size, 0x00);
		grape::serial::write(boost::asio::buffer(body), cred);
		//check if this thing has a formulary 

		//get the product data


		std::string name = mProductNameValue->GetValue().ToStdString();
		boost::trim(name);
		boost::to_lower(name);
		grape::product prod;
		prod.name          = std::move(name);
		prod.id            = boost::uuids::random_generator_mt19937{}();
		prod.serial_num    =  0; //change this 
		prod.generic_name  = std::move(mGenericNameValue->GetValue().ToStdString());
		prod.formulation   = std::move(mFormulationValue->GetStringSelection());
		prod.class_        = std::move(mClassValue->GetStringSelection().ToStdString());
		prod.barcode       = std::move(mScanProductString);
		prod.usage_info    = std::move(mDirForUseValue->GetValue().ToStdString());
		prod.indications   = std::move(mHealthConditionsValue->GetValue().ToStdString());
		prod.description   = std::move(mProductDescriptionValue->GetValue().ToStdString());
		prod.strength      = std::move(mStrengthValue->GetValue().ToStdString());
		prod.strength_type = std::move(mStrengthTypeValue->GetStringSelection().ToStdString());
		prod.package_size  = static_cast<std::uint64_t>(atoi(mPackageSizeValue->GetValue().ToStdString().c_str()));
		prod.sideeffects   = std::move(mSideEffectsValue->GetValue().ToStdString());

	
		auto fut = sess->req(http::verb::get, "/product/formulary/hasformulary", std::move(body));
		{
			wxBusyInfo wait("Checking for formulary\nPlease wait...");
			resp = fut.get();
		}
		grape::uid_t form_id;
		if (resp.result() == http::status::ok) {
			auto& body = resp.body();
			if (body.empty()) throw std::invalid_argument("expected a body");

			auto&& [id, buf] = grape::serial::read<grape::uid_t>(boost::asio::buffer(body));
			form_id = id;
		}
		else if ( resp.result() == http::status::not_found ) {
			//create a new formulary for 
			grape::formulary form;
			form.id           = boost::uuids::random_generator_mt19937{}();
			form.creator_id   = app.mPharmacyManager.pharmacy.id;
			form.created_by   = app.mPharmacyManager.account.username;
			form.access_level = grape::formulary_access_level::ACCESS_PRIVATE;
			form.usage_count  = 1ull;
			form.name         = app.mPharmacyManager.pharmacy.name;
			form.version      = "v1.0"s;
			const size_t size = grape::serial::get_size(form);
			grape::body_type bf(size + cred_size, 0x00);
			auto b  = grape::serial::write(boost::asio::buffer(bf), cred);
			auto b2 = grape::serial::write(b, form);

			fut = sess->req(http::verb::post, "/product/formulary/create", std::move(bf));
			{
				wxBusyInfo wait("Creating a formulary\nPlease wait...");
				resp = fut.get();
			}

			if (resp.result() != http::status::ok) {
				//cannot create formulary for some reason
				throw std::logic_error(app.ParseServerError(resp));
			}
			boost::fusion::at_c<0>(form_id) = form.id;
		}
		else if ( resp.result() != http::status::ok ) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		//add the product to formulary
		grape::collection_type<grape::product> collect; 
		boost::fusion::at_c<0>(collect).push_back(prod);
		const size_t usize    = grape::serial::get_size(form_id);
		const size_t psize    = grape::serial::get_size(collect);
		grape::body_type b(psize + usize + cred_size, 0x00);

		auto b1 = grape::serial::write(boost::asio::buffer(b), cred);
		auto b2 = grape::serial::write(b1, form_id);
		auto b3 = grape::serial::write(b2, collect);
		fut     = sess->req(http::verb::post, "/product/add", std::move(b));

		{
			wxBusyInfo wait("Adding product to formulary\nPlease wait...");
			resp = fut.get();
		}

		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		//add the product to pharmacy.
		grape::pharma_product pp;
		pp.branch_id       = cred.branch_id;
		pp.pharmacy_id     = cred.pharm_id;
		pp.product_id      = prod.id;
		pp.unitprice       = pof::base::data::currency_t(mSalePriceValue->GetValue().ToStdString());
		pp.costprice       = pof::base::data::currency_t(mCostPriceValue->GetValue().ToStdString());
		pp.stock_count     = static_cast<std::uint64_t>(atoi(mQunatityValue->GetValue().ToStdString().c_str()));
		pp.min_stock_count = 0ull;
		constexpr const size_t ppsize = grape::serial::get_size(pp);
		grape::body_type pb(ppsize + cred_size, 0x00);
		b1 = grape::serial::write(boost::asio::buffer(pb), cred);
		b2 = grape::serial::write(b1, pp);

		fut = sess->req(http::verb::post, "/product/addpharma"s, std::move(pb));
		{
			wxBusyInfo wait("Adding product to pharmacy\nPlease wait...");
			resp = fut.get();
		}

		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}
		return true;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(exp.what(), "Add product", wxICON_ERROR | wxOK);
		//EndModal(wxID_ABORT);
	}

	return false;
}

void ab::AddProductDialog::OnClose(wxCloseEvent& evt)
{
	EndModal(wxID_CANCEL);
}

void ab::AddProductDialog::OnScanProduct(wxCommandEvent& evt)
{
	while (1) {
		auto bar = wxGetTextFromUser("Please scan barcode", "Barcode");
		if (bar.empty()) return;

		//EAN-13 barcode format
		std::regex rex("^\d{13}$"s);
		if (!std::regex_match(bar.ToStdString(), rex)) {
			wxMessageBox("Invalid barcode");
		}
		else {
			mScanProductString = std::move(bar.ToStdString());
			break;
		}
	}
}

void ab::AddProductDialog::OnInventoryCheck(wxCommandEvent& evt)
{
}

void ab::AddProductDialog::OnMarkupCost(wxCommandEvent& evt)
{
}

wxArrayString ab::AddProductDialog::SetupSupplierName()
{
	return wxArrayString();
}
