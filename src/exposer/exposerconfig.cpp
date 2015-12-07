/*
 * exposerconfig.cpp
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */


#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>
#include <sstream>

#include <exception>
#include <Poco/Net/NetException.h>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <libconfig.h++>

#include "../other/file/log.h"
#include "../other/globals.h"


#include "pocowrapper.h"

#include "exposerconfig.h"

using namespace std;


ExposerConfig::ExposerConfig() {
}

ExposerConfig::~ExposerConfig() {
}

void ExposerConfig::initialize() {
	try {
		read_config_file();
		if (gloB->exposer_offline_mode == false) {
			handshake(gloB->exposer_userprofile_service_endpoint.c_str());
			handshake(gloB->exposer_gameevents_service_endpoint.c_str());
			get_sessionid();
			get_tokens();
		}
	}
	catch (std::exception &ex) {
		ostringstream tmpmsg;
		tmpmsg << "Could not initialize services. Going offline. Exception: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
		gloB->exposer_connection_is_setup = false;
		gloB->exposer_offline_mode = true;
	}
}

void ExposerConfig::finalize() {
	try {
		if (gloB->exposer_offline_mode == false && gloB->exposer_sessionid != "") {
			close_sessionid();
		}
	}
	catch (std::exception &ex) {
		ostringstream tmpmsg;
		tmpmsg << "Unexpected exception: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
}


void ExposerConfig::read_config_file() {
	string myconfig = gloB->exposer_config.get_rootful();
	if ( gloB->exposer_connection_is_setup == false ) {
	        libconfig::Config cfg;
	        try {
	            cfg.readFile(myconfig.c_str());
	        }
	        catch(const libconfig::FileIOException &fioex)
	        {
	            Log::log(Log::ERROR, "I/O error while reading network config file.");
	        }
	        catch(const libconfig::ParseException &pex)
	        {
	            ostringstream msg;
	            msg << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
	            Log::log(Log::ERROR, msg.str());
	        }
	        catch (std::exception &ex) {
	        	ostringstream tmpmsg;
	        	tmpmsg << "Could not open config file. Exception: " << ex.what();
	        	Log::log(Log::ERROR, tmpmsg.str());
	        }

	        //try to read the service endpoint

	        try
	        {
	            string tmp = cfg.lookup("gameevents_service.endpoint");
	            gloB->exposer_gameevents_service_endpoint = tmp;
	            if (gloB->exposer_gameevents_service_endpoint.empty() || gloB->exposer_gameevents_service_endpoint.size()==0) {
	                Log::log(Log::ERROR, "Could not find configuration for service endpoint.");
	            }
	            else {
	                //Make sure there's no trailing space in the service_endpoint string
	                if ( gloB->exposer_gameevents_service_endpoint[gloB->exposer_gameevents_service_endpoint.size()-1] == '/' )
	                    gloB->exposer_gameevents_service_endpoint = gloB->exposer_gameevents_service_endpoint.substr(0, gloB->exposer_gameevents_service_endpoint.size()-1);
	            }

	            string tmp10 = cfg.lookup("userprofile_service.endpoint");
	            gloB->exposer_userprofile_service_endpoint = tmp10;
	            if (gloB->exposer_userprofile_service_endpoint.empty() || gloB->exposer_userprofile_service_endpoint.size()==0) {
	            	Log::log(Log::ERROR, "Could not find configuration for user profile service endpoint.");
	            }
	            else {
	            	//Make sure there's no trailing space in the service_endpoint string
	            	if ( gloB->exposer_userprofile_service_endpoint[gloB->exposer_userprofile_service_endpoint.size()-1] == '/' )
	            		gloB->exposer_userprofile_service_endpoint = gloB->exposer_userprofile_service_endpoint.substr(0, gloB->exposer_userprofile_service_endpoint.size()-1);
	            }

	            string tmp2 = cfg.lookup("auth.clientid");
	            gloB->exposer_clientid = tmp2;

	            string tmp3 = cfg.lookup("auth.apikey");
	            gloB->exposer_apikey = tmp3;

	            bool tmp5 = cfg.lookup("record_local_file"); //Should i record a local file as well?
	            gloB->exposer_record_local_file = tmp5;


	            try {
	            	signed int tmp6;
	            	tmp6 = cfg.lookup("gameevents_service.max_number_attempts");
	            	//string tmp6_str = tmp6.str();
					//signed int tmp6_int = std::atoi(tmp6_str.c_str());
					if ( (tmp6 > 0) && (tmp6 < 100)) {
						gloB->exposer_max_number_attempts = tmp6;
					}
	            }
				catch (std::exception &ex) {
					ostringstream tmpmsg;
					tmpmsg << "Could not read max_number_attempts, using default value 3. Exception: " << ex.what();
					Log::log(Log::ERROR, tmpmsg.str());
					gloB->exposer_max_number_attempts = 3;
				}

				bool tmp7 = cfg.lookup("offline_mode"); //Offline mode does not try to contact the services and records everything in a local file
				gloB->exposer_offline_mode = tmp7;

				string tmp8 = cfg.lookup("user.username");
				gloB->exposer_username = tmp8;

				string tmp9 = cfg.lookup("user.password");
				gloB->exposer_password = tmp9;



				gloB->exposer_connection_is_setup = true;
	        }
	        catch(const libconfig::SettingNotFoundException &nfex)
	        {
	            Log::log(Log::ERROR, "Required settings not found.");
	        }
	        catch(std::exception &ex)
	        {
	        	ostringstream tmpmsg;
	        	tmpmsg << "Exception while trying to get settings from config file. " << ex.what();
	        	Log::log(Log::ERROR, tmpmsg.str());
	        }
	    } else {} //Connection is already set up, skipping...
}

void ExposerConfig::get_sessionid() {

	bool success = false;
	signed int counter = 1;

	if (not gloB->exposer_offline_mode) {

		while ((counter <= gloB->exposer_max_number_attempts) and (not success) and  (not gloB->exposer_offline_mode) )
		{
			try {
				get_sessionid_attempt();
				success = true;
			}
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Exception in getsessionid attempt #"<< counter <<": " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				counter++;
				//break;
			}

		}
	}
}

void ExposerConfig::get_sessionid_attempt() {

	string resource = "sessions";

	//prepare request body
	ostringstream payload_ss;
	payload_ss << "{\"username\" : \"" << gloB->exposer_username << "\", ";
	payload_ss << "\"password\" : \"" << gloB->exposer_password << "\"";
	payload_ss << "}";

	string request_url = gloB->exposer_userprofile_service_endpoint + "/" + resource;
	string payload = payload_ss.str();


	PocoWrapper requester(Poco::Net::HTTPRequest::HTTP_POST, request_url, "", payload);
	requester.execute();

	int response_status = requester.get_response_status();
	string response_body;
	response_body = requester.get_response_body();

	//HTTP 200 OK
	if (response_status == 200 || response_status == 201) {
		gloB->exposer_sessionid = extract_sessionid(response_body);

		if (gloB->exposer_sessionid == "") {
			gloB->exposer_offline_mode = true;
		}
	}
	//HTTP 401 UNAUTHORIZED
	else if (response_status == 401) {
		Log::log(Log::INFO, "Username provided in config file was not authorized to get a sessionid. Running in offline mode.");
		gloB->exposer_offline_mode = true;
	}
	else {
		//response failed
		ostringstream tmpmsg;
		tmpmsg << "Failed to get sessionid. Status [user profile service]: " << response_status << ".";
		throw Poco::Net::NetException(tmpmsg.str());
		//Log::log(Log::INFO,  tmpmsg.str() );
		//gloB->exposer_offline_mode = true;
	}


}

void ExposerConfig::handshake(string service, string resource) {
	/*
	 * Tries to connect to the service first to make sure it is up and running. If not,
	 * switches game to offline mode.
	 */

	bool success = false;
	signed int counter = 1;

	string request_url = service + "/" + resource;

	while ((counter <= gloB->exposer_max_number_attempts) and (not success))
	{
		ostringstream tmpmsg;
		tmpmsg << "Ping attempt number " << counter <<" of " << gloB->exposer_max_number_attempts << ".";
		Log::log(Log::INFO, tmpmsg.str());

		try {
			ping(request_url);
			success = true;
		}
		catch (std::exception &ex) {
			ostringstream tmpmsg;
			tmpmsg << "Exception in attempt #"<< counter << " : " << ex.what();
			Log::log(Log::ERROR, tmpmsg.str());
			counter++;
			//break;
		}
	}

	if (not success) {
		ostringstream tmpmsg;
		tmpmsg << "Failed handshake after maximum attempts. Going offline.";
		Log::log(Log::ERROR, tmpmsg.str());
		gloB->exposer_offline_mode=true;
	}

}

void ExposerConfig::ping(string url) {

	PocoWrapper requester(Poco::Net::HTTPRequest::HTTP_GET, url);
	requester.execute();

	int response_status = requester.get_response_status();

	//HTTP 200 OK
	if (response_status == 200) {
		ostringstream tmpmsg;
		tmpmsg << "Ping successful, service at " << url << " up and running.";
		Log::log(Log::INFO, tmpmsg.str());
	}
	else {
		ostringstream tmpmsg;
		tmpmsg << "Ping failed response: " << response_status;
		throw Poco::Net::NetException(tmpmsg.str());
	}
}

void ExposerConfig::get_tokens() {
	get_gameevents_token(gloB->exposer_max_number_attempts);
}

void ExposerConfig::get_gameevents_token(signed int number_of_attempts) {
	bool success = false;
	bool is_token_set = false;
	if ( not gloB->exposer_token.empty() and gloB->exposer_token.size()>0 ) {
		is_token_set = true;
	}
	signed int counter = 1;

	if (gloB->exposer_connection_is_setup and not gloB->exposer_offline_mode and is_token_set == false) {
		while ((counter <= number_of_attempts) and (not success) and (not gloB->exposer_offline_mode))
		{
			//ostringstream tmpmsg;
			//tmpmsg << "Attempt to get token #" << counter << " of " << number_of_attempts << ".";
			//Log::log(Log::INFO, tmpmsg.str());

			try {
				get_gameevents_token_attempt();
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

void ExposerConfig::get_gameevents_token_attempt() {

	const string resource = "token";
	string request_url = gloB->exposer_gameevents_service_endpoint + "/" + resource;

	//prepare request body
	ostringstream request_body_ss;
	request_body_ss << "{\"clientid\" : \"" << gloB->exposer_clientid << "\", ";
	request_body_ss << "\"apikey\" : \"" << gloB->exposer_apikey << "\", ";
	request_body_ss << "\"sessionid\" : \"" << gloB->exposer_sessionid << "\"";
	request_body_ss << "}";

	string request_body;
	request_body = request_body_ss.str();


	PocoWrapper poco_requester(Poco::Net::HTTPRequest::HTTP_POST, request_url, "", request_body);
	poco_requester.execute();

	int response_status;
	response_status = poco_requester.get_response_status();
	string response_body;
	response_body = poco_requester.get_response_body();

	if (response_status == 200 ) {
		gloB->exposer_token = extract_token(response_body);
	}
	else if (response_status == 401) {
		Log::log(Log::INFO, "Could not authenticate. Raising not authenticated exception.");
		//gloB->exposer_token = "";
		throw Poco::Net::NotAuthenticatedException("Could not authenticate");
	}
	else if (response_status == 500) {
		Log::log(Log::INFO, "500 error in server. Raising net exception.");
		//gloB->exposer_token = "";
		throw Poco::Net::NetException("500 error in server... try again?");
	}
	else {
		//response failed
		ostringstream tmpmsg;
		tmpmsg << "Failed to get token. Status: " << response_status;
		throw Poco::Net::NetException(tmpmsg.str());
		//gloB->exposer_token = "";
	}

	//ostringstream response_stream;
	//poco_requester.get_response(response_stream);
}

string ExposerConfig::extract_sessionid(string json_string) {
	try
	{
		//Try to extract sessionid
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(json_string);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		string tmpsessionid = object->get("id");

		ostringstream tmpmsg;
		tmpmsg << " ===== Got a sessionid: " << tmpsessionid;
		Log::log(Log::INFO, tmpmsg.str());

		return tmpsessionid;

	}
	catch (Poco::Exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "Exception when trying to read sessionid: " << ex.displayText();
		Log::log(Log::ERROR, tmpmsg.str());
		return "";
	}
}

string ExposerConfig::extract_token(string response_string) {
	try
	{
		//Try to extract token
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(response_string);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		string tmptoken = object->get("token");

		ostringstream tmpmsg;
		//tmpmsg << " ===== Got a token: " << tmptoken;
		tmpmsg << " ===== Got a token.";
		Log::log(Log::INFO, tmpmsg.str());

		return tmptoken;
	}
	catch (std::exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "Exception when trying to read token: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
		return "";
	}

}

void ExposerConfig::close_sessionid() {

	bool success = false;
	signed int counter = 1;

	if (not gloB->exposer_offline_mode) {

		while ((counter <= gloB->exposer_max_number_attempts) and (not success) and  (not gloB->exposer_offline_mode) )
		{
			try {
				close_sessionid_attempt();
				success = true;
			}
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Exception in closesessionid attempt #"<< counter <<": " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				counter++;
				//break;
			}

		}
	}
}

void ExposerConfig::close_sessionid_attempt() {

	string resource = "sessions";

	//prepare request body
	ostringstream payload_ss;
	payload_ss << "{\"username\" : \"" << gloB->exposer_username << "\", ";
	payload_ss << "\"password\" : \"" << gloB->exposer_password << "\"";
	payload_ss << "}";

	string request_url = gloB->exposer_userprofile_service_endpoint + "/" + resource + "/" + gloB->exposer_sessionid ;
	string payload = payload_ss.str();


	PocoWrapper requester(Poco::Net::HTTPRequest::HTTP_DELETE, request_url, "", payload);
	requester.execute();

	int response_status = requester.get_response_status();
	string response_body;
	response_body = requester.get_response_body();

	//HTTP 200 OK
	if (response_status == 200 || response_status == 202 || response_status == 204 ) {
		gloB->exposer_sessionid = "";
	}
	//HTTP 401 UNAUTHORIZED
	else if (response_status == 401) {
		Log::log(Log::INFO, "Not authorized to update this sessionid. Resetting sessionid.");
		gloB->exposer_sessionid = "";
		//gloB->exposer_offline_mode = true;
	}
	//HTTP 404 NOT FOUND
	else if (response_status == 404) {
		Log::log(Log::INFO, "Sessionid does not exist anymore. Resetting sessionid.");
		gloB->exposer_sessionid = "";
		//gloB->exposer_offline_mode = true;
	}
	else {
		//response failed
		ostringstream tmpmsg;
		tmpmsg << "Failed to delete sessionid. Status [user profile service]: " << response_status << ".";
		throw Poco::Net::NetException(tmpmsg.str());
		//Log::log(Log::INFO,  tmpmsg.str() );
		//gloB->exposer_offline_mode = true;
	}


}
