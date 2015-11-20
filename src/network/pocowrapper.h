/*
 * pocowrapper.h
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
//#include <Poco/Exception.h>
//#include <Poco/JSON/Parser.h>
//#include <Poco/Dynamic/Var.h>
//#include <Poco/Net/HTTPRequest.h>
//#include <Poco/Net/NetException.h>
//#include <Poco/Path.h>
//#include <Poco/URI.h>
//#include <Poco/StreamCopier.h>

class PocoWrapper {

	public:
		PocoWrapper(std::string url, std::string payload, std::string auth_token);
		void post();
		void get_response(std::ostringstream& output_stream);
		Poco::Net::HTTPResponse::HTTPStatus get_response_status();

	private:
		std::string url;
		std::string payload;
		std::string auth_token;
		std::string request_path;
		Poco::Net::HTTPClientSession *client_session;
		Poco::Net::HTTPResponse response;
		Poco::Net::HTTPResponse::HTTPStatus status;
};




