#include "PrintManager.hpp"
#include "Application.hpp"


ab::PrintManager::PrintManager()
{
	mPrintData = std::make_unique<wxPrintData>();
	mPrintDialogData = std::make_unique<wxPrintDialogData>(*mPrintData);

	mPrintDialogData->EnableSelection(true);
	mPrintDialogData->EnablePageNumbers(true);
	mPrintDialogData->SetMinPage(1);
	mPrintDialogData->SetMaxPage(2);
	mPrintDialogData->SetFromPage(1);
	mPrintDialogData->SetToPage(1);
	mPrintDialogData->SetAllPages(true);
}

void ab::PrintManager::PrinterSetup()
{
	if (!mPrintDialogData) return;
	(*mPageSetupData) = *mPrintData;

	wxPageSetupDialog pageSetupDialog(nullptr, mPageSetupData.get());
	pageSetupDialog.ShowModal();

	(*mPrintData) = pageSetupDialog.GetPageSetupDialogData().GetPrintData();
	(*mPageSetupData) = pageSetupDialog.GetPageSetupDialogData();
}

void ab::PrintManager::PrintReceipt(size_t type)
{
	gPrintState = type;
	po = new ab::Printout(mPrintDialogData.get());
	po->mFooterMessage = "THANK YOU FOR YOUR PATRONAGE!";
	if (wxGetApp().bShowPreviewOnSale) {
		po2 = new ab::Printout(mPrintDialogData.get());
		po2->mFooterMessage = "THANK YOU FOR YOUR PATRONAGE!";
		Preview(wxGetApp().mMainFrame, po, po2);
	}
	else {
		PrintJob(wxGetApp().mMainFrame, po);
		delete po;
		if (po2 != nullptr) delete po2;
	}
}

void ab::PrintManager::PrintJob(wxWindow* parent, wxPrintout* printout)
{
	wxPrinter printer(mPrintDialogData.get());
	if (printout) {
		if (!printer.Print(parent, printout, wxGetApp().bShowPrintPrompt)) {
			if (GetLastError() == wxPRINTER_ERROR) {
				spdlog::error("Problem printing");
				wxMessageBox("Problem printing", "Printing", wxICON_ERROR | wxOK);
			}
			else if (GetLastError() == wxPRINTER_CANCELLED) {
				spdlog::error("Print cancelled");
			}
			printSig(false, gPrintState);
		}
		else {
			printSig(true, gPrintState);
			(*mPrintData) = printer.GetPrintDialogData().GetPrintData();
		}
	}
}

void ab::PrintManager::Preview(wxWindow* parent, wxPrintout* previewout, wxPrintout* printout)
{
	//copy the print out so that the copy is deleted.
	wxPrintDialogData printDialogData(*mPrintData);
	//cannnout use the same pointer this is weird
	wxPrintPreview* preview = new wxPrintPreview(previewout, printout, &printDialogData);
	if (!preview->Ok())
	{
		delete preview;
		wxMessageBox(wxT("There was a problem previewing.\nPerhaps your current printer is not set correctly?"),
			wxT("Previewing"), wxOK);
		return;
	}

	wxPreviewFrame* frame = new wxPreviewFrame(preview, parent,
		wxT("Label Print Preview"), wxDefaultPosition, wxSize(878, 689));
	frame->Centre(wxBOTH);
	frame->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& evt) {
		printSig(true, gPrintState);
		evt.Skip();
	});

	frame->Initialize();
	frame->Show();
}
