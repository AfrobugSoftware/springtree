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

		grape::collection_type<grape::pharmacy> GetPharmacies();
		grape::collection_type<grape::branch> GetBranches(const boost::uuids::uuid& pharm, size_t start, size_t limit);
		grape::collection_type<grape::pharmacy> SearchPharmacies(const std::string& str);
		void GetPharmacyAddress(); //uses the pharmacy id;
		grape::address GetBranchAddress() const;

		std::string GetAccountTypeAsString() const;
		std::string GetPharmacyTypeAsString() const;

		grape::credentials mSessionCredentials;
		grape::address address;
		grape::branch branch;
		grape::pharmacy pharmacy;
		grape::account account;
	};
};