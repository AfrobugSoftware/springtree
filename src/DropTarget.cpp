#include "DropTarget.hpp"

ab::DropTarget::DropTarget(ab::DataObject* obj, TargetSignal::slot_type&& slot)
{
	SetDataObject(obj);
	mTargetSignal.connect(std::forward<TargetSignal::slot_type>(slot));
}

void ab::DropTarget::OnLeave()
{
}

wxDragResult ab::DropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if (!GetData()) {
		spdlog::error("Cannot get data on drop");
		return wxDragNone;
	}
	auto dataobj = static_cast<ab::DataObject*>(GetDataObject());
	if (!dataobj) return wxDragNone;
	mTargetSignal(*dataobj);
	return def;
}
