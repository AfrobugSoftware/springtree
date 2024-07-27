#pragma once
#include <boost/noncopyable.hpp>
#include "Grape.hpp"


#include <spdlog/spdlog.h>

#include <wx/msgdlg.h>

namespace ab {
	class PharmacyManager : public boost::noncopyable
	{
	public:
		PharmacyManager() = default;
		~PharmacyManager() {}

		bool CreatePharmacy();
		bool CreateBranch();

		grape::credentials mSessionCredentials;
		grape::address address;
		grape::branch branch;
		grape::pharmacy pharmacy;
	};
};