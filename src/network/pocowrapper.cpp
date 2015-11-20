/*
 * pocowrapper.cpp
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include "../other/myalleg.h" //needs to come first

#include "pocowrapper.h"
#include "../other/file/log.h"

#include <string>
#include <sstream>

#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSessionFactory.h>
#include <Poco/Net/HTTPSessionInstantiator.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>

using namespace std;



//PocoWrapper::PocoWrapper() {
//	url = "";
//	payload = "";
//	auth_token = "";
//	status = HTTPResponse::HTTP_INTERNAL_SERVER_ERROR;
//	//URI request_uri(url);
//	// prepare path
//	//request_path = request_uri.getPathAndQuery();
//	//if (request_path.empty()) request_path = "/";
//	//HTTPClientSession client_session(request_uri.getHost(), request_uri.getPort());
//}

PocoWrapper::PocoWrapper(string myurl, string mypayload, string myauth_token) {
	url = myurl;
	payload = mypayload;
	auth_token = myauth_token;
	status = Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR;

	// prepare path
	Poco::URI request_uri(url);
	request_path = request_uri.getPathAndQuery();
	if (request_path.empty()) {
		request_path = "/";
	}
	Poco::Net::HTTPSessionFactory::defaultFactory().registerProtocol("http", new Poco::Net::HTTPSessionInstantiator);
	client_session = Poco::Net::HTTPSessionFactory::defaultFactory().createClientSession(request_uri);
}

Poco::Net::HTTPResponse::HTTPStatus PocoWrapper::get_response_status() {
	return status;
}


void PocoWrapper::get_response(ostringstream& output_stream) {
	std::istream &is = client_session->receiveResponse(response);
	Poco::StreamCopier::copyStream(is, output_stream);
	status = response.getStatus();
}



void PocoWrapper::post() {

	try {
		//In Windows we need to initialize the network
		#ifdef _WIN32
				Poco::Net::initializeNetwork();
		#endif


		// prepare request
				Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, request_path);

		if  (not (payload.empty() || payload.size()==0))
		{
			//Is there a non-empty token?
			if  (not auth_token.empty() && auth_token.size()>0 )
			{
				request.set("X-AUTH-TOKEN", auth_token);
			}
			request.setContentType("application/json");
			request.setKeepAlive(true);
			request.setContentLength( payload.length() );

			//Log::log(Log::INFO, "Sending request...");

			//send request
			client_session->sendRequest(request) << payload;
		}
		else {
			//throw badly formatted request exception
			client_session->reset();
			//throw Poco::Exception("Bad request.");
			Log::log(Log::ERROR, "Bad request");
		}
	}
	catch (Poco::Net::ConnectionRefusedException& ex) {
		ostringstream tmpmsg;
		tmpmsg << "POCO connection refused: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
	catch (Poco::Net::NetException& ex) {
		ostringstream tmpmsg;
		tmpmsg << "POCO Net exception: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
	catch (Poco::Exception& ex) {
		ostringstream tmpmsg;
		tmpmsg << "POCO exception: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
	catch (std::exception &ex) {
		ostringstream tmpmsg;
		tmpmsg << "Unexpected exception: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
}
