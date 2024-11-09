#include "Printout.h"
#include "Application.hpp"

ab::Printout::Printout(wxPrintDialogData* data, const std::string& title)
: wxPrintout(title){
    mPrintDialogData = data;
    m_paper_type = static_cast<wxPaperSize>(wxGetApp().mPaperType);
    PerformPageSetup(wxGetApp().bShowPageSetup);
}

bool ab::Printout::OnPrintPage(int page)
{
	int state = wxGetApp().mPrintManager.gPrintState;
	switch (state)
	{
	case ab::PrintManager::LABELS:
		DrawLabelPrint(page);
		break;
	case ab::PrintManager::RECEIPT:
	case ab::PrintManager::REPRINT_RECEIPT:
		DrawSalePrint();
		break;
	case ab::PrintManager::ORDERlIST:
		//DrawOrderList();
		break;
	default:
		return false;
	}
	return true;
}

bool ab::Printout::HasPage(int page)
{
	return (page == 1);
}

bool ab::Printout::OnBeginDocument(int startPage, int endPage)
{
	if (!wxPrintout::OnBeginDocument(startPage, endPage))
		return false;

	wxSize paperSize = m_page_setup.GetPaperSize();  // in millimeters

	// still in millimeters
	float large_side = std::max(paperSize.GetWidth(), paperSize.GetHeight());
	float small_side = std::min(paperSize.GetWidth(), paperSize.GetHeight());

	float large_side_cm = large_side / 10.0f;  // in centimeters
	float small_side_cm = small_side / 10.0f;  // in centimeters

	float ratio = float(large_side - topMargin - bottomMargin) /
		float(small_side - leftMargin - rightMargin);

	m_coord_system_width = (int)((small_side_cm - topMargin / 10.f -
		bottomMargin / 10.0f) * 30);
	m_coord_system_height = m_coord_system_width * ratio;
    return true;
}

void ab::Printout::GetPageInfo(int* minPage, int* maxPage, int* selPageFrom, int* selPageTo)
{
	*minPage     = this->minPage;
	*maxPage     = this->maxPage;
	*selPageFrom = this->selPageFrom;
	*selPageTo   = this->selPageTo;
}

void ab::Printout::PerformPageSetup(bool showSetup)
{
	// don't show page setup dialog, use default values
	wxPrintData printdata;
	printdata.SetPrintMode(wxPRINT_MODE_PRINTER);
	printdata.SetOrientation(wxPORTRAIT);
	printdata.SetNoCopies(wxGetApp().copies);
	printdata.SetPaperId(wxGetApp().paperSize);

	m_page_setup = wxPageSetupDialogData(printdata);
	m_page_setup.SetMarginTopLeft(wxPoint(wxGetApp().leftMargin, wxGetApp().topMargin));
	m_page_setup.SetMarginBottomRight(wxPoint(wxGetApp().rightMargin, wxGetApp().bottomMargin));

	if (showSetup)
	{
		wxPageSetupDialog dialog(NULL, &m_page_setup);
		if (dialog.ShowModal() == wxID_OK)
		{

			m_page_setup = dialog.GetPageSetupData();
			wxGetApp().paperSize = m_page_setup.GetPrintData().GetPaperId();

			wxPoint marginTopLeft     = m_page_setup.GetMarginTopLeft();
			wxPoint marginBottomRight = m_page_setup.GetMarginBottomRight();
			wxGetApp().copies         = m_page_setup.GetPrintData().GetNoCopies();
			wxGetApp().leftMargin     = marginTopLeft.x;
			wxGetApp().rightMargin    = marginBottomRight.x;
			wxGetApp().topMargin      = marginTopLeft.y;
			wxGetApp().bottomMargin   = marginBottomRight.y;
		}
	}
}

size_t ab::Printout::WritePageHeader(wxPrintout* printout, wxDC* dc, const wxString& text, double mmToLogical)
{
	auto& app = wxGetApp();

	int border = 5;
	int xPos = wxGetApp().leftMargin, yPos = wxGetApp().topMargin;
	int lineLength = m_coord_system_width;
	int lineHeight = 18;

	wxFont font(wxFONTSIZE_SMALL, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString);

	dc->SetFont(wxGetApp().mReceiptFontSettings.GetChosenFont());


	wxCoord xExtent = 0, yExtent = 0;
	dc->GetTextExtent(text, &xExtent, &yExtent);
	lineHeight = yExtent;

	dc->DrawText(text, lineLength / 2 - xExtent / 2, yPos + border);

	std::string addy = app.mPharmacyManager.GetAddressAsString();
	if (!addy.empty())
	{
		yPos += lineHeight + border;
		dc->GetTextExtent(addy, &xExtent, &yExtent);
		dc->DrawText(addy, lineLength / 2 - xExtent / 2, yPos + border);
	}
	//print contact
	std::string contact = app.mPharmacyManager.GetContactAsString();
	if (!contact.empty())
	{
		yPos += yExtent + border;
		dc->GetTextExtent(contact, &xExtent, &yExtent);
		dc->DrawText(contact, lineLength / 2 - xExtent / 2, yPos + border);
	}

	yPos += yExtent + border + 5;
	wxRect rect(xPos, yPos, lineLength, lineHeight);
	std::chrono::sys_days date; 
	
	auto saleView = app.mMainFrame->GetSaleView();
	
	dc->DrawLabel(fmt::format("Date/Time: {:%Y-%m-%d %H:%M:%S}", saleView->mReceipt.date), rect, wxALIGN_LEFT);

	//account name and status
	yPos += yExtent + border;
	rect.SetPosition(wxPoint(xPos, yPos));
	auto& acc = wxGetApp().mPharmacyManager.account;
	std::string accName = fmt::format("{}, {} {}", app.mPharmacyManager.GetAccountTypeAsString(), acc.last_name, acc.first_name);
	dc->DrawLabel(accName, rect, wxALIGN_LEFT);

	//sale id
	yPos += yExtent + border;

	rect.SetPosition(wxPoint(xPos, yPos));
	dc->DrawLabel(fmt::format("id: {}", boost::lexical_cast<std::string>(saleView->mReceipt.id)), rect, wxALIGN_LEFT);

	yPos += yExtent + border;

	dc->SetPen(wxPenInfo(*wxBLACK).Style(wxPENSTYLE_SHORT_DASH));
	dc->DrawLine(xPos, yPos + border,     lineLength, yPos + border);
	dc->DrawLine(xPos, yPos + border + 5, lineLength, yPos + border + 5);

	//draw titile
	const std::string title = "INVOICE";
	dc->GetTextExtent(title, &xExtent, &yExtent);
	yPos += border + 10;
	dc->DrawText(title, lineLength / 2 - xExtent / 2, yPos);

	yPos += yExtent + border + 2;

	return yPos;
}

size_t ab::Printout::WriteSaleData(double mToLogical, size_t y)
{
	wxDC* dc = GetDC();
	auto& app = wxGetApp();
	auto saleView = app.mMainFrame->GetSaleView();

	int border = 5;
	int xPos = 0, yPos = y, xExtent = 0, yExtent = 18;
	int lineLength = m_coord_system_width;
	dc->GetTextExtent("X", &xExtent, &yExtent);
	int lineHeight = yExtent;

	//tite
	wxRect rect(xPos + border, yPos, lineLength, lineHeight - border);
	dc->DrawLabel("Quantity", rect, wxALIGN_LEFT);
	dc->DrawLabel("Product", rect, wxALIGN_CENTER);
	dc->DrawLabel("Sub-amount", rect, wxALIGN_RIGHT);

	yPos += lineHeight + 2;
	rect.SetPosition(wxPoint(xPos + border, yPos + border));

	auto& cachefont = dc->GetFont();
	dc->SetFont(wxGetApp().mReceiptFontSettings.GetChosenFont());

	auto sales = saleView->GetCurrentModel();
	if (!sales) throw std::runtime_error("failed to get current model");
	for (auto& sale : *sales)
	{
		auto& v = boost::fusion::at_c<2>(sale);

		dc->DrawLabel(fmt::to_string(v[1].GetLong()), rect, wxALIGN_LEFT);
		dc->DrawLabel(v[0].GetString(), rect, wxALIGN_CENTER);
		dc->DrawLabel(v[4].GetString(), rect, wxALIGN_RIGHT);

		yPos += lineHeight + 2;
		rect.SetPosition(wxPoint(xPos + border, yPos + border));
	}

	dc->SetFont(cachefont);
	//footer
	yPos += lineHeight;
	dc->SetPen(wxPenInfo(*wxBLACK).Style(wxPENSTYLE_SHORT_DASH));
	dc->DrawLine(xPos, yPos + border, lineLength, yPos + border);
	dc->DrawLine(xPos, yPos + border + 5, lineLength, yPos + border + 5);
	//dc->SetFont(mInoiceHeaderFont);
	std::string totalA = fmt::format("TOTAL: {:cu}",saleView->mReceipt.total);
	dc->GetTextExtent(totalA, &xExtent, &yExtent);
	yPos += border + 10;
	wxRect TotalRect(xPos, yPos + border, lineLength - border, yExtent);

	dc->DrawLabel(totalA, TotalRect, wxALIGN_RIGHT);

	yPos += yExtent + border;
	//dc->SetFont(mFooterFont);
	dc->SetPen(wxPenInfo(*wxBLACK).Style(wxPENSTYLE_SHORT_DASH));
	dc->DrawLine(xPos, yPos + border, lineLength, yPos + border);
	dc->GetTextExtent(mFooterMessage, &xExtent, &yExtent);
	dc->DrawText(mFooterMessage, lineLength / 2 - xExtent / 2, yPos + border + 2);


	//
	yPos += yExtent + border;
	std::string copyRight = "Powered by PharmaOffice";
	dc->GetTextExtent(copyRight, &xExtent, &yExtent);
	xPos = lineLength / 2;//int(((((pageWidthMM - leftMargin - rightMargin) / 2.0) + leftMargin) * mmToLogical) - (xExtent / 2.0));
	dc->DrawText(copyRight, lineLength / 2 - xExtent / 2, yPos + border);
	dc->DrawText(" ", lineLength / 2 - xExtent / 2, yPos + border);


	return yPos;
}

size_t ab::Printout::WritePageHeaderSmall(wxPrintout* printout, wxDC* dc, const wxString& text, double mmToLogical)
{
	auto& app = wxGetApp();
	auto saleView = app.mMainFrame->GetSaleView();

	int border = 0;
	int xPos = wxGetApp().leftMargin, yPos = wxGetApp().topMargin;
	int lineLength = m_coord_system_width;
	int lineHeight = 18;


	dc->SetFont(wxGetApp().mReceiptFontSettings.GetChosenFont());

	wxCoord xExtent = 0, yExtent = 0;
	dc->GetTextExtent(text, &xExtent, &yExtent);
	lineHeight = yExtent;

	dc->DrawText(text, lineLength / 2 - xExtent / 2, yPos + border);
	std::string addy = app.mPharmacyManager.GetAddressAsString();
	yPos += lineHeight + border;
	dc->GetTextExtent(addy, &xExtent, &yExtent);
	dc->DrawText(addy, lineLength / 2 - xExtent / 2, yPos + border);

	//print contact
	std::string contact = app.mPharmacyManager.GetContactAsString();

	if (!contact.empty()) {
		yPos += yExtent + border;
		dc->GetTextExtent(contact, &xExtent, &yExtent);
		dc->DrawText(contact, lineLength / 2 - xExtent / 2, yPos + border);
	}
	//date/time
	yPos += yExtent + border + 5;
	wxRect rect(xPos, yPos, lineLength, lineHeight);

	dc->DrawLabel(fmt::format("{:%Y-%m-%d %H:%M:%S}", saleView->mReceipt.date), rect, wxALIGN_LEFT);

	//account name and status
	yPos += yExtent + border;
	rect.SetPosition(wxPoint(xPos, yPos));
	auto& acc = app.mPharmacyManager.account;
	std::string accName = fmt::format("{} {}", acc.last_name, acc.first_name);
	dc->DrawLabel(accName, rect, wxALIGN_LEFT);

	//sale id
	yPos += yExtent + border;
	rect.SetPosition(wxPoint(xPos, yPos));
	dc->DrawLabel(fmt::format("id: {}", boost::lexical_cast<std::string>(saleView->mReceipt.id)), rect, wxALIGN_LEFT);

	yPos += yExtent + border + 10;

	dc->SetPen(wxPenInfo(*wxBLACK).Style(wxPENSTYLE_SHORT_DASH));
	dc->DrawLine(xPos, yPos + border, lineLength, yPos + border);
	dc->DrawLine(xPos, yPos + border + 5, lineLength, yPos + border + 5);

	//draw titile
	const std::string title = "Invoice";
	dc->GetTextExtent(title, &xExtent, &yExtent);
	yPos += border + 5;
	dc->DrawText(title, lineLength / 2 - xExtent / 2, yPos);

	yPos += yExtent + border + 2;

	return yPos;
}

size_t ab::Printout::WriteSaleDataSmall(double mToLogical, size_t y)
{
	wxDC* dc = GetDC();
	auto& app = wxGetApp();
	auto saleView = app.mMainFrame->GetSaleView();

	int border = 5;
	int xPos = 0, yPos = y, xExtent = 0, yExtent = 18;
	int lineLength = m_coord_system_width; //rightMarginLogical - leftMarginLogical;
	dc->GetTextExtent("X", &xExtent, &yExtent);

	int lineHeight = yExtent;

	//tite
	wxRect rect(xPos, yPos, lineLength, lineHeight - border);

	dc->DrawLabel("Qty Product", rect, wxALIGN_LEFT);
	dc->DrawLabel("Sub-amount", rect, wxALIGN_RIGHT);

	yPos += lineHeight + 2;
	rect.SetPosition(wxPoint(xPos + border, yPos + border));

	dc->SetFont(wxGetApp().mReceiptFontSettings.GetChosenFont());
	auto sales = saleView->GetCurrentModel();
	if (!sales) //what happends here 
		throw std::system_error(std::make_error_code(std::errc::bad_address));
	for (auto& sale : *sales) {
		auto& v = boost::fusion::at_c<2>(sale);
		auto productText = fmt::format("{:d} {}", v[1].GetLong(), v[0].GetString().ToStdString());
		auto amountText = v[4].GetString().ToStdString();

		dc->GetTextExtent(amountText, &xExtent, &yExtent);
		int xSize = m_coord_system_width - xExtent - 10;
		dc->SetClippingRegion(wxRect{ xPos, yPos, xSize, lineHeight });

		dc->DrawText(productText, { xPos, yPos });

		dc->DestroyClippingRegion();

		dc->DrawText(amountText, xPos + xSize + 10, yPos);

		yPos += lineHeight + 2;
		rect.SetPosition(wxPoint(xPos + border, yPos + border));	
	}

	//footer
	yPos += lineHeight;
	dc->SetPen(wxPenInfo(*wxBLACK).Style(wxPENSTYLE_SHORT_DASH));
	dc->DrawLine(xPos, yPos + border, lineLength, yPos + border);
	dc->DrawLine(xPos, yPos + border + 5, lineLength, yPos + border + 5);
	std::string totalA = fmt::format("TOTAL: {:cu}", saleView->mReceipt.total);
	dc->GetTextExtent(totalA, &xExtent, &yExtent);
	yPos += border + 5;
	wxRect TotalRect(xPos, yPos + border, lineLength - border, yExtent);

	dc->DrawLabel(totalA, TotalRect, wxALIGN_RIGHT);

	yPos += yExtent + border;
	dc->SetPen(wxPenInfo(*wxBLACK).Style(wxPENSTYLE_SHORT_DASH));
	dc->DrawLine(xPos, yPos + border, lineLength, yPos + border);
	dc->GetTextExtent(mFooterMessage, &xExtent, &yExtent);
	dc->DrawText(mFooterMessage, lineLength / 2 - xExtent / 2, yPos + border + 2);


	//
	yPos += yExtent + border;
	std::string copyRight = "Powered by PharmaOffice";
	dc->GetTextExtent(copyRight, &xExtent, &yExtent);
	xPos = lineLength / 2;
	dc->DrawText(copyRight, lineLength / 2 - xExtent / 2, yPos + border);


	return yPos;

}

bool ab::Printout::DrawSalePrint()
{
	wxDC* dc = GetDC();
	auto& app = wxGetApp();


	FitThisSizeToPageMargins(dc->FromDIP(wxSize(m_coord_system_width, m_coord_system_height)), m_page_setup);
	wxRect fitRect = GetLogicalPageMarginsRect(m_page_setup);
	wxCoord xoff = (fitRect.width - m_coord_system_width) / 2;
	wxCoord yoff = (fitRect.height - m_coord_system_height) / 2;
	OffsetLogicalOrigin(dc->FromDIP(xoff), dc->FromDIP(yoff));

	dc->SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
	dc->SetBrush(*wxTRANSPARENT_BRUSH);
	//calculate the length of a line
	int y = 0;

	switch (m_paper_type)
	{
	case wxPAPER_NONE:
		y = WritePageHeaderSmall(this, dc, app.mPharmacyManager.pharmacy.name, logUnitsFactor);
		WriteSaleDataSmall(logUnitsFactor, y);
		break;
	case wxPAPER_LETTER:
	case wxPAPER_LEGAL:
	case wxPAPER_A4:
		y = WritePageHeader(this, dc, app.mPharmacyManager.pharmacy.name, logUnitsFactor);
		WriteSaleData(logUnitsFactor, y);
		break;
	case wxPAPER_A4SMALL:
		y = WritePageHeaderSmall(this, dc, app.mPharmacyManager.pharmacy.name, logUnitsFactor);
		WriteSaleDataSmall(logUnitsFactor, y);
		break;
	default:
		break;
	}

	return true;
}

bool ab::Printout::DrawLabelPrint(int page)
{
	return false;
}
