/*
 * pocowrapper.h
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>


class PocoWrapper {

	public:
		PocoWrapper(std::string url, std::string payload, std::string auth_token);
		void run();
		void post();
		std::string get_response_body();
		int get_response_status();

	private:
		//void get_response();
		//std::ostringstream& internal_stream;
		std::string response_body;
		std::string url;
		std::string payload;
		std::string auth_token;
		std::string request_path;
		Poco::Net::HTTPClientSession *client_session;
		Poco::Net::HTTPResponse response;
		int status;
};




