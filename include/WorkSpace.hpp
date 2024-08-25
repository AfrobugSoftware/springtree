#pragma once

#include <wx/treebase.h>
#include <wx/panel.h>
#include <wx/aui/auibook.h>
#include <boost/signals2.hpp>
#include "DropTarget.hpp"
#include "Grape.hpp"

namespace ab {
	class Workspace : public wxPanel {
	public:
		enum {
			WORKSPACEBOOK = wxID_HIGHEST + 3000
		};

		enum class notif {
			closed,
			opened,
			deleted,
			added,
			shown,
			hidden
		};
		boost::signals2::signal<void(notif, wxWindow*)> notifsignal;
		boost::signals2::signal<void(const wxTreeItemId&, const std::string& label)> mDropTreeSignal;

		Workspace(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(829, 644), long style = wxTAB_TRAVERSAL);
		virtual ~Workspace() = default;

		wxAuiNotebook& GetWorkspacebook() { return *mWorkspacebook; }
		size_t GetLastPage() const { return mWorkspacebook->GetPageCount() - 1; }
		size_t GetPageCount() const { return mWorkspacebook->GetPageCount(); }

		inline void SetImageList(wxImageList* imglist) { mWorkspacebook->SetImageList(imglist); }

		bool AddSpace(wxWindow* space, const std::string& name = "EMPTY", int img = -1);

	private:
		wxAuiNotebook* mWorkspacebook = nullptr;
		wxPanel* m_panel3 = nullptr;

		void OnWorkspaceClose(wxAuiNotebookEvent& evt);
		void OnDroppedTreeItem(const ab::DataObject& item);

		DECLARE_EVENT_TABLE()

	};
};
