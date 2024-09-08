#pragma once
#include <type_traits>
#include <functional>
#include <concepts>
#include <wx/valgen.h>
#include <wx/control.h>

namespace ab {
	template<std::derived_from<wxControl> Tctrl>
	class validator : public wxGenericValidator
	{
	public:
		using wxGenericValidator::wxGenericValidator;

		std::function<bool(Tctrl*)> OnValidate;
		virtual bool Validate(wxWindow* parent) override {
			Tctrl* control = dynamic_cast<Tctrl*>(parent);
			if (!control) return false;

			return OnValidate(control);
		}

	};
};