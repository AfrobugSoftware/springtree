#pragma once
#include <wx/app.h>
#include "serialiser.h"
#include "MainFrame.hpp"

namespace ab {
	class Application : public wxApp {
	public:
		Application();
		virtual ~Application() {}

		virtual bool OnInit() override;
		virtual int OnExit() override;

		ab::MainFrame* mMainFrame = nullptr;
	};
};

DECLARE_APP(ab::Application)