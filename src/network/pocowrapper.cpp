/*
 * pocowrapper.cpp
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include "../other/myalleg.h" //needs to come first

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

#include <Poco/ThreadLocal.h>

#include "pocowrapper.h"

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
	//status = Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR;
	status = 0;

	// prepare path
	Poco::URI request_uri(url);
	request_path = request_uri.getPathAndQuery();
	if (request_path.empty()) {
		request_path = "/";
	}
	Poco::Net::HTTPSessionFactory::defaultFactory().registerProtocol("http", new Poco::Net::HTTPSessionInstantiator);
	client_session = Poco::Net::HTTPSessionFactory::defaultFactory().createClientSession(request_uri);
}

int PocoWrapper::get_response_status() {
	return status;
}

string PocoWrapper::get_response_body() {
	return response_body;
}

void PocoWrapper::run() {
	send_package();
}

void PocoWrapper::send_package() {
	response_body = "";
	ostringstream internal_stream;

	//Log::log(Log::INFO, "I'm in run!");

	try {
		//Log::log(Log::INFO, "Before post");
		post();
		//Log::log(Log::INFO, "After post");
		std::istream &is = client_session->receiveResponse(response);
		Poco::StreamCopier::copyStream(is, internal_stream);
		status = response.getStatus();
		//Log::log(Log::INFO, "After get status");
		response_body = internal_stream.str();
		//Log::log(Log::INFO, "After get response body");

		//ostringstream tmpmsg;
		//tmpmsg << "Response body: " << response_body;
		//Log::log(Log::INFO, tmpmsg.str());

		if (status==0) {
			ostringstream tmpmsg;
			tmpmsg << "Error running service. Not enough time to get response?";
			Log::log(Log::ERROR, tmpmsg.str());
		}
		else if (status==401) {
			ostringstream tmpmsg;
			tmpmsg << "Not Authorized by service: 401 UNAUTHORIZED";
			Log::log(Log::ERROR, tmpmsg.str());
			//throw Poco::Net::NotAuthenticatedException("Not authorized by service.");
		}
	}
	catch (Poco::Net::NotAuthenticatedException& ex) {
		ostringstream tmpmsg;
		tmpmsg << "Not Authenticated: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
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



void PocoWrapper::post() {

	//Log::log(Log::INFO, "Inside post");
	//In Windows we need to initialize the network
	#ifdef _WIN32
		Poco::Net::initializeNetwork();
	#endif
		//Log::log(Log::INFO, "before creating request");

	// prepare request
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, request_path);

	//Log::log(Log::INFO, "request created");

	if  (not (payload.empty() || payload.size()==0))
	{
		//Log::log(Log::INFO, "payload not empty");
		//Is there a non-empty token?
		if  (not auth_token.empty() && auth_token.size()>0 )
		{
			//Log::log(Log::INFO, "token not empty");
			request.set("X-AUTH-TOKEN", auth_token);
		}
		request.setContentType("application/json");
		request.setKeepAlive(true);
		request.setContentLength( payload.length() );

		//Log::log(Log::INFO, "Sending request...");

		//send request
		client_session->sendRequest(request) << payload;
		//Log::log(Log::INFO, "request sent");
	}
	else {
		client_session->reset();
		ostringstream tmpmsg;
		tmpmsg << "Bad request.";
		Log::log(Log::ERROR, tmpmsg.str());
	}
}
