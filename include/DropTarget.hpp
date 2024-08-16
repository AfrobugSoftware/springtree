#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include <wx/dnd.h>
#include <boost/signals2.hpp>
#include "DataObject.hpp"
namespace ab {
	class DropTarget : public wxDropTarget
	{
	public:
		using TargetSignal = boost::signals2::signal<void(const ab::DataObject&)>;
		DropTarget(ab::DataObject* obj, TargetSignal::slot_type&& slot);
		~DropTarget() {}

		virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def) override
		{
			return OnDragOver(x, y, def);
		}

		virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
		virtual void OnLeave() override;
	private:
		TargetSignal mTargetSignal;
	};
};