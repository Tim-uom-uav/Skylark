#ifndef MISSION_CONTROL_HTTPS_GET_H_
#define MISSION_CONTROL_HTTPS_GET_H_

#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include "stdHeaders.h"

#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <sstream>

static std::stringstream https_get(std::string host, std::string get) {
	std::stringstream result;
	try {
		boost::asio::io_service io_service;

		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(host, "https");
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
		context.set_default_verify_paths();

		char request_[1024];
		boost::asio::streambuf reply_;
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_(io_service, context);
		socket_.set_verify_mode(boost::asio::ssl::verify_none); // Trust but don't verify
		boost::asio::connect(socket_.lowest_layer(), endpoint_iterator);
		socket_.handshake(boost::asio::ssl::stream_base::client);
		std::string raw = "GET " + get + " HTTP/1.0\r\n";
		raw += "Host: " + host + "\r\n";
		raw += "Connection: close\r\n";
		raw += "\r\n";
		size_t request_length = strlen(raw.c_str());
		std::copy(raw.c_str(), raw.c_str() + request_length, request_);
		boost::asio::write(socket_, boost::asio::buffer(request_, request_length));
		boost::system::error_code ec;
		while (ec.value() == 0) {
			boost::asio::read_until(socket_, reply_, "\r\n", ec);
			result << &reply_;
			if (ec.value() != 0) {
				break;
			}
		}
		io_service.run();
		io_service.reset();
		io_service.stop();
	}
	catch (std::exception& e) {
		std::cerr << "Main Exception: " << e.what() << "\n";
	}
	return result;
}

#endif // !MISSION_CONTROL_HTTPS_GET_H_
