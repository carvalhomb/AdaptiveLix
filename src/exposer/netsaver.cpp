/*
 * gameeventswrapper.cpp
 *
 *  Created on: 23 Nov 2015
 *      Author: maira
 */

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>
#include <sstream>
#include <exception>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>

#include "../other/file/log.h"
#include "../other/globals.h"

#include "pocowrapper.h"
#include "netsaver.h"

using namespace std;

NetworkSaver::NetworkSaver(GameData passed_event_data, int passed_num_attempts) {
	event_data = passed_event_data;
	num_attempts = passed_num_attempts;
}

void NetworkSaver::run() {
	if (gloB->exposer_offline_mode == false) {
		send_event(event_data, num_attempts);
	}
}


void NetworkSaver::send_event(GameData data, signed int number_of_attempts) {
	bool success = false;
	signed int counter = 1;

	//Format data
	std::string formatted_event_data;
	formatted_event_data = data.to_json();
	//formatted_event_data = data.to_xml();

	if (not gloB->exposer_offline_mode) {
		while ((counter <= number_of_attempts) and (not success) and (not gloB->exposer_offline_mode))
		{
			//ostringstream tmpmsg;
			//tmpmsg << "Attempt number " << counter;
			//Log::log(Log::INFO, tmpmsg.str());
			try {
				send_event_attempt(formatted_event_data);
				success = true;
			}
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Exception in attempt #"<< counter <<": " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				counter++;
				//break;
			}
		}
	}
}

void NetworkSaver::send_event_attempt(string formatted_event)
{
	string resource = "sessions/" + gloB->exposer_sessionid + "/events";
	string current_token = gloB->exposer_token;


	if (gloB->exposer_connection_is_setup == true && gloB->exposer_offline_mode==false) {
		if (current_token.empty() || current_token.size()==0) {
			//Log::log(Log::ERROR, "I don't have a token. Unsetting the connection so that it gets configured again.");
			Log::log(Log::ERROR, "I don't have a token. Going offline.");
			gloB->exposer_offline_mode=true;
		}
		else
		{
			//OK, I have a non-empty token

			//prepare request body
			time_t timestamp = time(0);
			ostringstream request_body_ss;
			request_body_ss << "{";
			request_body_ss << "\"timestamp\" : \"" << timestamp << "\", \n";
			request_body_ss << "\"events\" : " << formatted_event ;
			request_body_ss << "}";

			string request_body;
			request_body = request_body_ss.str();

			string request_url = "";
			request_url = gloB->exposer_gameevents_service_endpoint + "/" + resource;


			try {
				PocoWrapper poco_requester(Poco::Net::HTTPRequest::HTTP_POST, request_url, current_token, request_body);
				poco_requester.execute();

				//ostringstream response_stream;
				//poco_requester.get_response(response_stream);
			}
			catch(Poco::Net::NotAuthenticatedException &ex)
			{
				ostringstream tmpmsg;
				tmpmsg << "Token unauthorized. Resetting the token variable. Exception: " << ex.what();
				gloB->exposer_token = "";
				Log::log(Log::INFO, tmpmsg.str());
			}
			catch(Poco::Net::NetException &ex)
			{
//				ostringstream tmpmsg;
//				tmpmsg << "Server exception: " << ex.what();
//				Log::log(Log::INFO, tmpmsg.str());
				throw;
			}
			catch(std::exception &ex)
			{
				ostringstream tmpmsg;
				tmpmsg << "Unexpected exception while trying to send event. Going offline. Exception: " << ex.what();
				gloB->exposer_offline_mode = true;
				Log::log(Log::ERROR, tmpmsg.str());
			}

		}
	}
	else {
		ostringstream tmpmsg;
		tmpmsg << "Nothing to do, running offline.";
		Log::log(Log::INFO, tmpmsg.str());
	}
}
