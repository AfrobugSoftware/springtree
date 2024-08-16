#include "PharmacyManager.hpp"
#include "Application.hpp"

bool ab::PharmacyManager::CreatePharmacy()
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
		const size_t size = grape::serial::get_size(pharmacy) + grape::serial::get_size(address);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body),pharmacy);
		grape::serial::write(buf, address);

		auto fut = sess->req(http::verb::post, "/pharmacy/create", std::move(body));

		auto resp = fut.get();
		if (resp.result() != http::status::created) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto&& [temp, buff] = 
			grape::serial::read<grape::pharmacy>(boost::asio::buffer(resp.body()));
		pharmacy = std::move(temp);
		address.id = pharmacy.address_id;
		branch.pharmacy_id = pharmacy.id;
	}
	catch (std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(fmt::format("Failed to create pharmacy: {}", exp.what()),
			"FATAL ERROR", wxICON_ERROR | wxOK);
		return false;
	}
	return true;
}

bool ab::PharmacyManager::CreateBranch()
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
		const ssize_t size = grape::serial::get_size(branch) + grape::serial::get_size(address);
		grape::session::request_type::body_type::value_type body(size, 0x00);
		auto buf = grape::serial::write(boost::asio::buffer(body), branch);
		grape::serial::write(buf, address);

		auto fut = sess->req(http::verb::post, "/pharmacy/createbranch", std::move(body));

		auto resp = fut.get();
		if (resp.result() != http::status::created) {
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto&& [temp, rdbuf] = grape::serial::read<grape::branch>(boost::asio::buffer(resp.body()));
		branch = temp;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox(fmt::format("Failed to create pharmacy: {}", exp.what()),
			"FATAL ERROR", wxICON_ERROR | wxOK);
		return false;
	}
	return true;
}

grape::collection_type<grape::pharmacy> ab::PharmacyManager::GetPharmacies()
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), 
			app.mNetManager.ssl());
		auto fut = sess->req(http::verb::get, "/pharmacy/getpharmacies", {});
		auto resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& body = resp.body();
		if (body.empty()) throw std::logic_error("empty body recieved");

		auto&& [collection, buf] = grape::serial::read<grape::collection_type<grape::pharmacy>>(boost::asio::buffer(body));
		return collection;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		return grape::collection_type<grape::pharmacy>{};
	}
}

grape::collection_type<grape::branch> ab::PharmacyManager::GetBranches(const boost::uuids::uuid& pharm, size_t start, size_t limit)
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(),
			app.mNetManager.ssl());
	
		grape::page pg;
		pg.begin = start;
		pg.limit = limit;

		grape::uid_t id;
		boost::fusion::at_c<0>(id) = pharm;
	
		const size_t size = grape::serial::get_size(id) + grape::serial::get_size(pg);
		grape::session::request_type::body_type::value_type sbody(size, 0x00);

		auto buf = grape::serial::write(boost::asio::buffer(sbody), id);
		grape::serial::write(buf, pg);

		auto fut = sess->req(http::verb::get, "/pharmacy/getbranches",std::move(sbody), 30s);
		auto resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& body = resp.body();
		if (body.empty()) throw std::logic_error("empty body recieved");

		auto&& [collection, buf2] = grape::serial::read<grape::collection_type<grape::branch>>(boost::asio::buffer(body));
		return collection;


	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		return {};
	}
}

grape::collection_type<grape::pharmacy> ab::PharmacyManager::SearchPharmacies(const std::string& str)
{
	auto& app = wxGetApp();
	try {
		grape::string_t out;
		boost::fusion::at_c<0>(out) = str;
		grape::session::request_type::body_type::value_type body(grape::serial::get_size(out), 0x00);
		grape::serial::write(boost::asio::buffer(body), out);

		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		auto fut = sess->req(http::verb::search, "/pharmacy/search", std::move(body));


		//how do I wait for this thing?
		auto resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& rbody = resp.body();
		if (rbody.empty()) throw std::logic_error("Body should not be empty");

		auto&& [pharms, buf] = grape::serial::read<grape::collection_type<grape::pharmacy>>(boost::asio::buffer(rbody));
		return pharms;

	}catch(const std::exception& exp){
		spdlog::error(exp.what());
		return grape::collection_type<grape::pharmacy>{};
	}
}

void ab::PharmacyManager::GetPharmacyAddress()
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());
		
		grape::session::request_type::body_type::value_type sbody(pharmacy.id.begin(), pharmacy.id.end());
		auto fut = sess->req(http::verb::get, "/pharamcy/address", std::move(sbody), 30s);
		
		auto resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}
		auto& body = resp.body();
		if (body.empty()) throw std::logic_error("expected a body");

		auto&& [addy, b] = grape::serial::read<grape::address>(boost::asio::buffer(body));
		address = std::move(addy);
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
	}
}

grape::address ab::PharmacyManager::GetBranchAddress() const
{
	auto& app = wxGetApp();
	try {
		auto sess = std::make_shared<grape::session>(app.mNetManager.io(), app.mNetManager.ssl());

		grape::session::request_type::body_type::value_type body(branch.id.begin(), branch.id.end());
		auto fut = sess->req(http::verb::get, "/pharmacy/branch/address", std::move(body));
		grape::session::response_type resp = fut.get();
		if (resp.result() != http::status::ok) {
			throw std::logic_error(app.ParseServerError(resp));
		}

		auto& sbody = resp.body();
		if (sbody.empty()) throw std::logic_error("expected a body");

		auto&& [addy, b] = grape::serial::read<grape::address>(boost::asio::buffer(sbody));
		return addy;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		return grape::address();
	}
}

std::string ab::PharmacyManager::GetAccountTypeAsString() const
{
	static const std::array<std::string_view, 7> account_type_string = 
	{
		   "SUPERINTENDENT PHARMACIST", 
		   "PHARMTECH", 
		   "DISPENSER", 
		   "SALES ASSISTANT", 
		   "INTERN PHARMACIST", 
		   "STUDENT PHARMACIST",
			"MANAGER"
	};
	return std::string(account_type_string[static_cast<std::underlying_type_t<grape::account_type>>(account.type)]);
}

std::string ab::PharmacyManager::GetPharmacyTypeAsString() const
{
	constexpr static const std::array<std::string_view, 5> sPharamcyTypes =
	{
	   "COMMUNITY",
	   "HOSPITAL",
	   "INDUSTRY",
	   "WHOLESALE",
	   "EDUCATIONAL"
	};
	return std::string(sPharamcyTypes[static_cast<std::underlying_type_t<grape::branch_type>>(branch.type)]);
}
