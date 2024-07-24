#include "Application.hpp"

IMPLEMENT_APP(ab::Application)

ab::Application::Application() {
}

bool ab::Application::OnInit()
{
	mMainFrame = new ab::MainFrame(nullptr, wxID_ANY, wxDefaultPosition, wxSize(400,400));
	mMainFrame->Show();
	return true;
}

int ab::Application::OnExit()
{
	return 0;
}
