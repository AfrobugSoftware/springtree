#include <netmanager.h>

pof::base::net_manager::net_manager()
	: m_ssl{boost::asio::ssl::context_base::sslv23_server}{
	//auto ec = setupssl();
	//if (ec) {
	//	
	//}
	
	m_workgaurd = std::make_unique<net::executor_work_guard<net::io_context::executor_type>>(m_io.get_executor());
	m_threadvec.reserve(std::thread::hardware_concurrency());
	for (int i = 0; i < std::thread::hardware_concurrency() - 1; i++) {
		m_threadvec.emplace_back(std::move(std::thread{ static_cast<size_t(net::io_context::*)()>(&net::io_context::run), std::ref(m_io) }));
	}	
}

bool pof::base::net_manager::stop()
{
	m_workgaurd.reset(nullptr);
	m_io.stop();
	for(auto& t : m_threadvec)
		t.join();
	m_threadvec.clear();
	return true;
}

std::error_code pof::base::net_manager::setupssl()
{
	auto fp = std::filesystem::current_path() / "certs" / "certs.pem";

	//this would change
	try {
		boost::system::error_code ec;


		m_ssl.set_default_verify_paths();
		m_ssl.set_verify_mode(net::ssl::verify_peer);

		//when I get a certificate
		m_ssl.use_certificate_file(fp.string(), boost::asio::ssl::context::pem);
		m_ssl.use_rsa_private_key_file(fp.string(), boost::asio::ssl::context::pem);
		m_ssl.set_password_callback([](std::size_t len, boost::asio::ssl::context_base::password_purpose pp) -> std::string {
			return "zino";
			}, ec);
		return ec;
	}
	catch (const std::system_error& err) {
		return err.code();
	}
}


pof::base::net_manager::res_t pof::base::net_manager::bad_request(const std::string& err) const
{
	res_t res{ http::status::bad_request, 11 };

	res.set(http::field::server, USER_AGENT_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(true);

	js::json obj = js::json::object();
	obj["result_status"] = "failed"s;
	obj["result_message"] = err;



	auto ret = obj.dump();
	res_t::body_type::value_type value;
	value.resize(ret.size());
	std::copy(ret.begin(), ret.end(), value.begin());

	res.body() = value;
	res.prepare_payload();
	return res;
}

pof::base::net_manager::res_t pof::base::net_manager::server_error(const std::string& err) const
{
	res_t res{ http::status::internal_server_error, 11 };

	res.set(http::field::server, USER_AGENT_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(true);

	js::json obj = js::json::object();
	obj["result_status"] = "failed"s;
	obj["result_message"] = err;


	auto ret = obj.dump();
	res_t::body_type::value_type value;
	value.resize(ret.size());
	std::copy(ret.begin(), ret.end(), value.begin());

	res.body() = value;
	res.prepare_payload();
	return res;
}

pof::base::net_manager::res_t pof::base::net_manager::not_found(const std::string& err) const
{
	res_t res{ http::status::not_found, 11 };

	res.set(http::field::server, USER_AGENT_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(true);

	js::json obj = js::json::object();
	obj["result_status"] = "failed"s;
	obj["result_message"] = err;


	auto ret = obj.dump();

	res_t::body_type::value_type value;
	value.resize(ret.size());
	std::copy(ret.begin(), ret.end(), value.begin());


	res.body() = value;
	res.prepare_payload();
	return res;
}

pof::base::net_manager::res_t pof::base::net_manager::auth_error(const std::string& err) const
{
	res_t res{ http::status::unauthorized, 11 };
	res.set(http::field::server, USER_AGENT_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(true);

	js::json obj = js::json::object();
	obj["result_status"] = "failed"s;
	obj["result_message"] = err;


	auto ret = obj.dump();
	res_t::body_type::value_type value;
	value.resize(ret.size());
	std::copy(ret.begin(), ret.end(), value.begin());



	res.body() = value;
	res.prepare_payload();
	return res;
}

pof::base::net_manager::res_t pof::base::net_manager::unprocessiable(const std::string& err) const
{
	res_t res{ http::status::unprocessable_entity, 11 };
	res.set(http::field::server, USER_AGENT_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(true);

	js::json obj = js::json::object();
	obj["result_status"] = "failed"s;
	obj["result_message"] = err;


	auto ret = obj.dump();
	res_t::body_type::value_type value;
	value.resize(ret.size());
	std::copy(ret.begin(), ret.end(), value.begin());


	res.body() = value;
	res.prepare_payload();
	return res;
}

pof::base::net_manager::res_t pof::base::net_manager::timeout_error() const
{
	res_t res{ http::status::request_timeout, 11 };
	res.set(http::field::server, USER_AGENT_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(true);

	js::json obj = js::json::object();
	obj["result_status"] = "failed"s;
	obj["result_message"] = "operation timeout";


	auto ret = obj.dump();
	res_t::body_type::value_type value;
	value.resize(ret.size());
	std::copy(ret.begin(), ret.end(), value.begin());

	res.body() = value;
	res.prepare_payload();
	return res;
}

void pof::base::net_manager::run()
{
	boost::make_shared<listener>(*this, m_io, m_endpoint)->run();
	m_signals = std::make_shared<net::signal_set>(m_io, SIGINT, SIGTERM);
	m_signals->async_wait(
		[&](boost::system::error_code const&, int)
		{
			// Stop the io_context. This will cause run()
			// to return immediately, eventually destroying the
			// io_context and any remaining handlers in it.
			m_io.stop();
		});
}

void pof::base::net_manager::add_route(const std::string& target, callback&& cb)
{
	if (target.empty()) return;
	m_router.insert(target, std::forward<callback>(cb));
}

void pof::base::net_manager::listener::fail(beast::error_code ec, char const* what)
{
	// Don't report on canceled operations
	if (ec == net::error::operation_aborted)
		return;
	spdlog::error("Listner error :{}", what);
}

void pof::base::net_manager::listener::on_accept(beast::error_code ec, tcp::socket socket)
{
	if (ec) return fail(ec, "accept");

	//lunch a session
	spdlog::info("connected at {}", socket.remote_endpoint().address().to_string());
	boost::make_shared<httpsession>(manager,std::move(socket))->run();

	//schelde another accept
	 // The new connection gets its own strand
	acceptor_.async_accept(net::make_strand(ioc_),
		beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

pof::base::net_manager::listener::listener(net_manager& man,net::io_context& ioc, tcp::endpoint endpoint)
: manager(man), ioc_(ioc)
, acceptor_(ioc){
	beast::error_code ec;

	// Open the acceptor
	acceptor_.open(endpoint.protocol(), ec);
	if (ec){
		fail(ec, "open");
		return;
	}
	// Allow address reuse
	acceptor_.set_option(net::socket_base::reuse_address(true), ec);
	if (ec){
		fail(ec, "set_option");
		return;
	}
	// Bind to the server address
	acceptor_.bind(endpoint, ec);
	if (ec) {
		fail(ec, "bind");
		return;
	}
	// Start listening for connections
	acceptor_.listen(net::socket_base::max_listen_connections, ec);
	if (ec){
		fail(ec, "listen");
		return;
	}
}

void pof::base::net_manager::listener::run()
{
	// The new connection gets its own strand
	acceptor_.async_accept( net::make_strand(ioc_), 
		beast::bind_front_handler(&listener::on_accept,
			shared_from_this()));
}

void pof::base::net_manager::httpsession::fail(beast::error_code ec, char const* what)
{
	// Don't report on canceled operations
	if (ec == net::error::operation_aborted)
		return;
	spdlog::error("httpsession error :{} {}", what, ec.message());
}

void pof::base::net_manager::httpsession::do_read()
{
	parser.emplace();
	parser->body_limit(10000);
	stream_.expires_after(std::chrono::seconds(60));

	// Read a request
	http::async_read(
		stream_,
		buffer_,
		parser->get(),
		beast::bind_front_handler(
			&httpsession::on_read,
			shared_from_this()));
}

void pof::base::net_manager::httpsession::on_read(beast::error_code ec, std::size_t)
{
	// This means they closed the connection
	if (ec == http::error::end_of_stream){
		stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
		return;
	}
	// Handle the error, if any
	if (ec)
		return fail(ec, "read");

	auto& req = parser->get();
	const auto& target = req.target();
	
	auto rpath = boost::urls::parse_path(target);
	if (!rpath) {
		auto res = manager.bad_request("illegal request taget");
		auto self = shared_from_this();
		using response_type = typename std::decay<decltype(res)>::type;
		auto sp = boost::make_shared<response_type>(std::forward<decltype(res)>(res));
		http::async_write(stream_, *sp,
			[self, sp](
				beast::error_code ec, std::size_t bytes)
			{
				self->on_write(ec, bytes, sp->need_eof());
			});
		return;
	}
	stream_.expires_after(std::chrono::seconds(60));

	boost::urls::matches m;
	auto found = manager.m_router.find(rpath.value(), m);
	if (!found) {
		http::response<http::string_body> res{ http::status::not_found, 11 };

		res.set(http::field::server, USER_AGENT_STRING);
		res.set(http::field::content_type, "application/json");
		res.keep_alive(true);

		js::json obj = js::json::object();
		obj["result_status"] = "failed"s;
		obj["result_message"] = "The resource '" + std::string(target) + "' was not found.";

		res.body() = obj.dump();
		res.prepare_payload();

		using response_type = typename std::decay<decltype(res)>::type;
		auto sp = boost::make_shared<response_type>(std::forward<decltype(res)>(res));
		auto self = shared_from_this();
		http::async_write(stream_, *sp,
			[self, sp](
				beast::error_code ec, std::size_t bytes)
			{
				self->on_write(ec, bytes, sp->need_eof());
			});
	}
	else {
		boost::asio::co_spawn(stream_.get_executor(), (*found)(std::move(parser->get()), std::move(m)), [self = shared_from_this()](std::exception_ptr ptr, pof::base::net_manager::res_t res) {
			using response_type = typename std::decay<decltype(res)>::type;
		auto sp = boost::make_shared<response_type>(std::forward<decltype(res)>(res));
		http::async_write(self->stream_, *sp,
			[self = self->shared_from_this(), sp](
				beast::error_code ec, std::size_t bytes)
			{
				self->on_write(ec, bytes, sp->need_eof());
			});
		});
	}
}

void pof::base::net_manager::httpsession::on_write(beast::error_code ec, std::size_t, bool close)
{
	// Handle the error, if any
	if (ec)
		return fail(ec, "write");

	if (close)
	{
		// This means we should close the connection, usually because
		// the response indicated the "Connection: close" semantic.
		stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
		return;
	}

	// Read another request
	do_read();
}

pof::base::net_manager::httpsession::httpsession(net_manager& man, tcp::socket&& socket)
: manager(man), stream_(std::move(socket)){

}

void pof::base::net_manager::httpsession::run()
{
	do_read(); //begin the reading
}
