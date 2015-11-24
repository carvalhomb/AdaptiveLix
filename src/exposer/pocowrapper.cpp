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
#include <exception>

#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSessionFactory.h>
#include <Poco/Net/HTTPSessionInstantiator.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>


#include "pocowrapper.h"

using namespace std;



PocoWrapper::PocoWrapper(string mymethod, string myurl, string myauth_token, string mypayload) {
	method = mymethod;
	url = myurl;
	payload = mypayload;
	auth_token = myauth_token;
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


void PocoWrapper::execute() {
	response_body = "";
	ostringstream internal_stream;

	//In Windows we need to initialize the network
	#ifdef _WIN32
		Poco::Net::initializeNetwork();
	#endif


	try {
		// prepare request
		Poco::Net::HTTPRequest request(method, request_path);

		//Is there a non-empty token?
		if  (not auth_token.empty() && auth_token.size()>0 )
		{
			//Log::log(Log::INFO, "token not empty");
			request.set("X-AUTH-TOKEN", auth_token);
		}

		//is there a request body? Is this a POST request?
		if  (not (payload.empty() || payload.size()==0) && (method == Poco::Net::HTTPRequest::HTTP_POST))
		{
			request.setContentType("application/json");
			request.setKeepAlive(true);
			request.setContentLength( payload.length() );
			//Log::log(Log::INFO, payload);
			//send request
			client_session->sendRequest(request) << payload;

		}
		else {
			//Not a POST request, send request without payload
			client_session->sendRequest(request);
		}

		std::istream &is = client_session->receiveResponse(response);
		Poco::StreamCopier::copyStream(is, internal_stream);
		status = response.getStatus();
		response_body = internal_stream.str();

//		ostringstream tmpmsg;
//		tmpmsg << "Response body: " << response_body;
//		Log::log(Log::INFO, tmpmsg.str());

		if (status==0) {
			ostringstream tmpmsg;
			tmpmsg << "Error running service.";
			Log::log(Log::ERROR, tmpmsg.str());
		}
		else if (status==401) {
			ostringstream tmpmsg;
			tmpmsg << "Not Authorized by service: 401 UNAUTHORIZED";
			Log::log(Log::ERROR, tmpmsg.str());
		}
		else if (status==500) {
			ostringstream tmpmsg;
			tmpmsg << "Error in service: 500 INTERNAL SERVER ERROR";
			Log::log(Log::ERROR, tmpmsg.str());
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

