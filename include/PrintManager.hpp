#pragma once
#include <wx/print.h>
#include <wx/msgdlg.h>
#include <wx/printdlg.h>
#include "Printout.h"
#include <boost/signals2.hpp>

#include <optional>
namespace ab {
	class PrintManager
	{
	public:
		//print types
		enum {
			RECEIPT,
			REPRINT_RECEIPT,
			LABELS,
			ORDERlIST,
		};

		PrintManager();
		~PrintManager() = default;

		void PrinterSetup();
		void PrintReceipt(size_t type, std::optional<std::vector<grape::sale_display>> sa = std::nullopt);
		//void PrintLabels(const std::vector<pof::LabelInfo>& labels, wxWindow* parent);
		void PrintJob(wxWindow* parent, wxPrintout* printjob);
		void Preview(wxWindow* parent, wxPrintout* previeout, wxPrintout* printout);

		size_t gPrintState;
		boost::signals2::signal<void(bool, size_t)> printSig;
		std::unique_ptr<wxPrintDialogData> mPrintDialogData;
		std::unique_ptr<wxPageSetupData> mPageSetupData;
		std::unique_ptr<wxPrintData> mPrintData;

		ab::Printout* po = nullptr;
		ab::Printout* po2 = nullptr;

	};
};