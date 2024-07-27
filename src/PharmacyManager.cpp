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
			throw std::logic_error(resp.reason().data());
		}

		auto&& [temp, buff] = 
			grape::serial::read<grape::pharmacy>(boost::asio::buffer(resp.body()));
		pharmacy = std::move(temp);
		address.id = pharmacy.address_id;
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
		const ssize_t size = grape::serial::get_size()


	}
	catch (const std::exception& exp) {
		spdlog::error(exp.what());
		wxMessageBox("Failed to create pharmacy",
			"FATAL ERROR", wxICON_ERROR | wxOK);
		return false;
	}
	return true;
}
