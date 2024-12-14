#include "Reports.hpp"
#include "Application.hpp"
BEGIN_EVENT_TABLE(ab::Reports, wxPanel)
	EVT_DATAVIEW_ITEM_ACTIVATED(ab::Reports::ID_REPORT_LIST, ab::Reports::OnListActivated)
	EVT_TOOL(ab::Reports::ID_BACK, ab::Reports::OnBack)
	EVT_TOOL(ab::Reports::ID_DOWNLOAD_EXCEL, ab::Reports::OnDownloadExcel)
END_EVENT_TABLE()

ab::Reports::Reports(wxWindow* parent, wxWindowID id, const wxPoint& position, const wxSize& size, long style)
	: wxPanel(parent, id, position, size, style), mManager{this, ab::AuiTheme::AUIMGRSTYLE} {

	CreateToolbar();
	CreatePanels();
	CreateListView();
	CreateReportViews();
	mManager.Update();
}

void ab::Reports::CreateToolbar()
{

}

void ab::Reports::CreateListView()
{
}

void ab::Reports::CreatePanels()
{
}

void ab::Reports::CreateReportViews()
{

}

void ab::Reports::OnListActivated(wxDataViewEvent& evt)
{

}

void ab::Reports::OnDownloadExcel(wxCommandEvent& evt)
{
}

void ab::Reports::OnBack(wxCommandEvent& evt)
{
}
