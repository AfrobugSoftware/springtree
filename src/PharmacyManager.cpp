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

		auto fut = sess->req(http::verb::put, "/pharmacy/createbranch", std::move(body));

		auto resp = fut.get();
		if (resp.result() != http::status::created) {
			throw std::logic_error(resp.reason().data());
		}
		auto&& [temp, rdbuf] = grape::serial::read<grape::branch>(boost::asio::buffer(resp.body()));
		branch = temp;
	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox("Failed to create pharmacy",
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
