#pragma once
#include <wx/font.h>
#include <wx/print.h>
#include "Grape.hpp"

#include <vector>
#include <functional>
namespace ab {
	class Printout : public wxPrintout
	{
	public:
		Printout(wxPrintDialogData* data, const std::string& title = "PharmaOffice receipt");
		virtual ~Printout() = default;
		virtual bool OnPrintPage(int page) override;
		virtual bool HasPage(int page) override;
		virtual bool OnBeginDocument(int startPage, int endPage) override;
		virtual void GetPageInfo(int* minPage, int* maxPage, int* selPageFrom, int* selPageTo) override;

		void PerformPageSetup(bool showSetup = false);

		size_t WritePageHeader(wxPrintout* printout, wxDC* dc, const wxString& text, double mmToLogical);
		size_t WriteSaleData(double mToLogical, size_t ypos);

		size_t WritePageHeaderSmall(wxPrintout* printout, wxDC* dc, const wxString& text, double mmToLogical);
		size_t WriteSaleDataSmall(double mToLogical, size_t ypos);

		//size_t WriteOrderListSmall();
		//size_t WriteOrderList();


		bool DrawSalePrint();
		bool DrawLabelPrint(int page);
	//	bool DrawOrderList();

		std::string mFooterMessage;
		int startPage = 1, endPage = 1, pageNum = 1;
		wxFont mPharmacyNameFont;
		wxFont mHeaderFont;
		wxFont mBodyFont;
		wxFont mFooterFont;
		wxFont mContactFont;
		wxFont mInoiceHeaderFont;

		int leftMargin   = 2;
		int topMargin    = 2;
		int	bottomMargin = 2;
		int rightMargin  = 2;
		int pageCount    = 1;
		int minPage      = 1;
		int maxPage      = 1;
		int selPageFrom  = 1;
		int selPageTo    = 1;
		double logUnitsFactor = 0.0;
		
		//std::vector<LabelInfo> mLabels;
		wxPrintDialogData* mPrintDialogData;

		wxPageSetupDialogData m_page_setup;
		std::vector<grape::sale_display> mSaleCache;
		/** the type of paper (letter, A4, etc...) */
		wxPaperSize m_paper_type;
		int m_coord_system_width, m_coord_system_height;
	};
};