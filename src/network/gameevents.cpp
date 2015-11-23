#include "gameevents.h"
#include "gamedata.h"

//#include "../other/myalleg.h" //needs to come first

#include <string>
#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/NetException.h>
#include <Poco/ThreadPool.h>
//#include "Poco/Thread.h"
#include <Poco/RunnableAdapter.h>
#include <Poco/ThreadLocal.h>


#include <libconfig.h++>

#include "../other/file/log.h"

#include "pocowrapper.h"

using namespace std;

//Game events service info
string GameEvents::gameevents_service_endpoint = "";
string GameEvents::clientid = "";
string GameEvents::apikey = "";
string GameEvents::token = "";
string GameEvents::sessionid = "";

//Userprofile service info
string GameEvents::userprofile_service_endpoint = "";
string GameEvents::username = "";
string GameEvents::password = "";

bool GameEvents::connection_is_setup = false;
bool GameEvents::offline_mode = false;
bool GameEvents::record_local_file = true;
signed int GameEvents::max_number_attempts = 3;

//////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////



void GameEvents::get_sessionid() {
	if (GameEvents::offline_mode==false) {

		//Is the connection set up? If not, run configure()
		if (connection_is_setup == false) {
			Log::log(Log::INFO, "Connection is not set up when trying to get sessionid. Running 'configure()'...");
			GameEvents::configure();
		}

		//And now, is the connection set up? And is the token empty?
		if (connection_is_setup == true) {
			Log::log(Log::INFO, "Connection is set up, sending request...");
			string resource = "sessions";

			//prepare request body
			ostringstream payload_ss;
			payload_ss << "{\"username\" : \"" << GameEvents::username << "\", ";
			payload_ss << "\"password\" : \"" << GameEvents::password << "\"";
			payload_ss << "}";

			string request_url = GameEvents::userprofile_service_endpoint + "/" + resource;
			string payload = payload_ss.str();

			try {
				PocoWrapper poco_requester(request_url, payload, "");

//				Poco::RunnableAdapter<PocoWrapper> poco_runnable(poco_requester, &PocoWrapper::run);
//				Poco::ThreadPool::defaultPool().start(poco_runnable);
//				Poco::ThreadPool::defaultPool().joinAll();
				poco_requester.send_package();

				int response_status = poco_requester.get_response_status();
				string response_body;
				response_body = poco_requester.get_response_body();

				//HTTP 200 OK
				if (response_status == 200) {
					GameEvents::sessionid = extract_sessionid(response_body);

					if (GameEvents::sessionid == "") {
						GameEvents::offline_mode = true;
					}
				}
				//HTTP 401 UNAUTHORIZED
				else if (response_status == 401) {
					Log::log(Log::INFO, "Username provided in config file was not authorized to get a sessionid. Running in offline mode.");
					GameEvents::offline_mode = true;
				}
				else {
					//response failed
					ostringstream tmpmsg;
					tmpmsg << "Failed to get sessionid. Response status from user profile service: " << response_status << ". Running in offline mode.";
					Log::log(Log::INFO,  tmpmsg.str() );
					GameEvents::offline_mode = true;
				}
			}
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Unexpected exception: " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
			}
		}
		else
		{
			Log::log(Log::ERROR, "Could not set up the connection settings.");
		}

	}

}



void GameEvents::send_event(GameData event_data) {
	if (not GameEvents::connection_is_setup) {
		Log::log(Log::INFO, "Connection is not yet set up. Running 'configure()'...");
		GameEvents::configure();
		if (not GameEvents::max_number_attempts || GameEvents::max_number_attempts < 0 || GameEvents::max_number_attempts > 100) {
			Log::log(Log::ERROR, "Invalid value for max_number_attempts variable. Using '3' as default.");
			GameEvents::max_number_attempts = 3;
		}
	}
	GameEvents::send_event(event_data, GameEvents::max_number_attempts);
}



void GameEvents::send_event(GameData data, signed int number_of_attempts) {
	bool success = false;
	signed int counter = 1;

	//Format data
	std::string formatted_event_data;
	formatted_event_data = GameEvents::format_event_data(data);

	if (not GameEvents::offline_mode) {
		while ((counter <= number_of_attempts) and (not success) and (not GameEvents::offline_mode))
		{
			//ostringstream tmpmsg;
			//tmpmsg << "Attempt number " << counter;
			//Log::log(Log::INFO, tmpmsg.str());
			try {
				GameEvents::send_event_attempt(formatted_event_data);
				success = true;
			}
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Exception in attempt #"<< counter <<": " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				break;
			}
		}
	}
	else {
		Log::log(Log::INFO, "Working in offline mode.");
	}


	if (GameEvents::offline_mode || GameEvents::record_local_file) {
			GameEvents::log_event_locally(data);
	}

}



//////////////////////////////////////////////////////////////////////
// Protected methods
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////

void GameEvents::configure()
{
    if ( GameEvents::connection_is_setup == false ) {
        libconfig::Config cfg;
        //Try to open the config file
        try {
        	//string
        	string myconfig = gloB->file_netevents_config.get_rootful();
        	Log::log(Log::INFO, myconfig);
            cfg.readFile(gloB->file_netevents_config.get_rootful().c_str());
        }
        catch(const libconfig::FileIOException &fioex)
        {
            Log::log(Log::ERROR, "I/O error while reading file.");
        }
        catch(const libconfig::ParseException &pex)
        {
            //string msg = "Parse error at " << string(pex.getFile()) << ":" << pex.getLine() << " - " << pex.getError();
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
            GameEvents::gameevents_service_endpoint = tmp;
            if (GameEvents::gameevents_service_endpoint.empty() || GameEvents::gameevents_service_endpoint.size()==0) {
                Log::log(Log::ERROR, "Could not find configuration for service endpoint.");
            }
            else {
                //Make sure there's no trailing space in the service_endpoint string
                if ( GameEvents::gameevents_service_endpoint[gameevents_service_endpoint.size()-1] == '/' )
                    GameEvents::gameevents_service_endpoint = GameEvents::gameevents_service_endpoint.substr(0, GameEvents::gameevents_service_endpoint.size()-1);
            }

            string tmp10 = cfg.lookup("userprofile_service.endpoint");
            GameEvents::userprofile_service_endpoint = tmp10;
            if (GameEvents::userprofile_service_endpoint.empty() || GameEvents::userprofile_service_endpoint.size()==0) {
            	Log::log(Log::ERROR, "Could not find configuration for user profile service endpoint.");
            }
            else {
            	//Make sure there's no trailing space in the service_endpoint string
            	if ( GameEvents::userprofile_service_endpoint[userprofile_service_endpoint.size()-1] == '/' )
            		GameEvents::userprofile_service_endpoint = GameEvents::userprofile_service_endpoint.substr(0, GameEvents::userprofile_service_endpoint.size()-1);
            }

            string tmp2 = cfg.lookup("auth.clientid");
            GameEvents::clientid = tmp2;

            string tmp3 = cfg.lookup("auth.apikey");
            GameEvents::apikey = tmp3;

            bool tmp5 = cfg.lookup("record_local_file"); //Should i record a local file as well?
            GameEvents::record_local_file = tmp5;


            try {
            	signed int tmp6;
            	tmp6 = cfg.lookup("gameevents_service.max_number_attempts");
            	//string tmp6_str = tmp6.str();
				//signed int tmp6_int = std::atoi(tmp6_str.c_str());
				if ( (tmp6 > 0) && (tmp6 < 100)) {
					GameEvents::max_number_attempts = tmp6;
				}
            }
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Could not read max_number_attempts, using default value 3. Exception: " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				GameEvents::max_number_attempts = 3;
			}

			bool tmp7 = cfg.lookup("offline_mode"); //Offline mode does not try to contact the services and records everything in a local file
			GameEvents::offline_mode = tmp7;

			string tmp8 = cfg.lookup("user.username");
			GameEvents::username = tmp8;

			string tmp9 = cfg.lookup("user.password");
			GameEvents::password = tmp9;



            connection_is_setup = true;
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



string GameEvents::get_token()
{

    //TODO: send a session id when creating the token so that the service can associate
    // the game events to the correct session id. I need to decide the format of this
    // session id. get from UP service?

    const string resource = "token";

    //Is the connection set up? If not, run configure()
    if (connection_is_setup == false) {
    	Log::log(Log::INFO, "Connection is not set up. Running 'configure()'...");
    	GameEvents::configure();
    }

    //And now, is the connection set up? And is the token empty?
    if (connection_is_setup == true) {
    	if ( GameEvents::token.empty() || GameEvents::token.size()==0 ) {
    		//prepare request body
    		ostringstream request_body_ss;
    		request_body_ss << "{\"clientid\" : \"" << GameEvents::clientid << "\", ";
    		request_body_ss << "\"apikey\" : \"" << GameEvents::apikey << "\", ";
    		request_body_ss << "\"sessionid\" : \"" << GameEvents::sessionid << "\"";
    		request_body_ss << "}";

    		string request_body;
    		request_body = request_body_ss.str();

    		string request_url = GameEvents::gameevents_service_endpoint + "/" + resource;

    		PocoWrapper poco_requester(request_url, request_body, "");

//    		Poco::RunnableAdapter<PocoWrapper> poco_runnable(poco_requester, &PocoWrapper::run);
//    		Poco::ThreadPool::defaultPool().start(poco_runnable);
//    		Poco::ThreadPool::defaultPool().joinAll();
    		poco_requester.send_package();

    		int response_status;
    		response_status = poco_requester.get_response_status();
    		string response_body;
    		response_body = poco_requester.get_response_body();

    		if (response_status == 200 ) {
    			//response: OK
//    			ostringstream tmpmsg;
//    			tmpmsg << "Successfully sent token request, status " << response_status;
//    			tmpmsg << ". This is the body: ";
//    			tmpmsg << response_body;
//    			Log::log(Log::INFO, tmpmsg.str());
    			GameEvents::token = GameEvents::extract_token(response_body);
    		}
    		else if (response_status == 401) {
    			Log::log(Log::INFO, "Token unauthorized. Resetting the token variable.");
    			GameEvents::token = "";
    			//throw Poco::Net::NotAuthenticatedException("Token unauthorized");
    		}
    		else {
    			//response failed
    			ostringstream tmpmsg;
    			tmpmsg << "Failed to commit game event. Status: " << response_status;
    			Log::log(Log::INFO,  tmpmsg.str() );
    			GameEvents::token = "";
    			//throw Poco::Net::NetException(response.getReason());
    		}
    	}
    	else
    	{
    		//Log::log(Log::INFO, "I already have a token. Continuing, so that I return the existing one...");
    	}
    }
    else
    {
    	Log::log(Log::ERROR, "Could not set up the connection settings.");
    }

    return GameEvents::token;
}



void GameEvents::send_event_attempt(string event)
{
	string resource = "sessions/" + GameEvents::sessionid + "/events";
	string current_token;

	try {
		//Log::log(Log::INFO, "Send event attempt, first making sure I have a token.");
		current_token = GameEvents::get_token();
	}
	catch (std::exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "Unexpected exception while trying to get token: " << ex.what();
		Log::log(Log::ERROR,  tmpmsg.str());
	}

	if (connection_is_setup == false) {
		Log::log(Log::ERROR, "Could not set up the connection.");
	}
	else
	{
		if (current_token.empty() || current_token.size()==0) {
			Log::log(Log::ERROR, "Could not get a token.");
		}
		else
		{
			//OK, I have a non-empty token

			//prepare request body
			time_t timestamp = time(0);
			ostringstream request_body_ss;
			request_body_ss << "{";
			request_body_ss << "\"timestamp\" : \"" << timestamp << "\", ";
			request_body_ss << "\"gameevent\" : \"" << event << "\"";
			request_body_ss << "}";

			string request_body;
			request_body = request_body_ss.str();

			string request_url = "";
			request_url = GameEvents::gameevents_service_endpoint + "/" + resource;

			//Log::log(Log::INFO, "Going to try sending the request via runnable adapter");
			PocoWrapper poco_requester(request_url, request_body, current_token);
			//Log::log(Log::INFO, "Initialized object.");

			Poco::RunnableAdapter<PocoWrapper> poco_runnable(poco_requester, &PocoWrapper::run);
			//Log::log(Log::INFO, "created adapted object");

			//Poco::ThreadPool::defaultPool().start(poco_runnable);

			try {

				Poco::ThreadPool::defaultPool().start(poco_runnable);
				//Log::log(Log::INFO, "started thread");
				Poco::ThreadPool::defaultPool().joinAll();

				//PocoWrapper poco_requester(request_url, request_body, current_token);
				//poco_requester.send_package();

				//ostringstream response_stream;
				//poco_requester.get_response(response_stream);
			}
			catch(Poco::Net::NotAuthenticatedException &ex)
			{
				ostringstream tmpmsg;
				tmpmsg << "Token unauthorized. Resetting the token variable. Exception: " << ex.what();
				GameEvents::token = "";
				Log::log(Log::INFO, tmpmsg.str());
			}
			catch(std::exception &ex)
			{
				ostringstream tmpmsg;
				tmpmsg << "Exception while trying to send event. Going offline. Exception: " << ex.what();
				GameEvents::offline_mode = true;
				Log::log(Log::ERROR, tmpmsg.str());
			}

		}
	}
}


bool GameEvents::file_exists(std::string filename) {
    ifstream f(filename.c_str(), ifstream::in);
    return !f.fail(); // using good() could fail on empty files
}

void GameEvents::log_event_locally(GameData event_data) {

	try {
		string filename = gloB->file_events_output.get_rootful();
		string formatted_line = GameEvents::format_event_data_csv(event_data);
		ofstream myfile;

		if (GameEvents::file_exists(filename)) {
			//Log::log(Log::INFO, "File exists, appending...");
			myfile.open(filename.c_str(), std::ios_base::app);
			myfile << formatted_line;
			myfile.close();
		}
		else {
			//Log::log(Log::INFO, "File does NOT exists, creating it and preparing header...");

			//create file and write first line, then the formatted line
			myfile.open(filename.c_str());
			myfile << "sessionid, timestamp, action, level, update, ";
			myfile << "seconds, which_lix, lix_required, lix_saved, ";
			myfile << "skills_used, seconds_required \n";
			myfile << formatted_line;
			myfile.close();
		}
	}
	catch (std::exception &ex) {
		ostringstream tmpmsg;
		tmpmsg << "Exception while trying to write local file. " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
	}
}





string GameEvents::format_event_data(GameData data)
{
	ostringstream data_sstr;

	data_sstr << "<event>";
	data_sstr << "<timestamp>" << data.timestamp << "</timestamp>";
	data_sstr << "<action>" << data.action << "</action>";
	if (data.level != "0") data_sstr <<  "<level>" << data.level << "</level>";
	if (data.update >= 0) data_sstr << "<update>" << data.update << "</update>";
	if (data.seconds >= 0) data_sstr << "<seconds>" << data.seconds << "</seconds>";
	if (data.which_lix >= 0 ) data_sstr << "<which>" << data.which_lix << "</which>";
	if (data.lix_required >= 0 && (data.action == "ENDLEVEL")) data_sstr << "<lix_required>" << data.lix_required << "</lix_required>";
	if (data.lix_saved >= 0 && (data.action == "ENDLEVEL")) data_sstr << "<lix_saved>" << data.lix_saved << "</lix_saved>";
	if (data.skills_used >= 0 && (data.action == "ENDLEVEL")) data_sstr << "<skills_used>" << data.skills_used << "</skills_used>";
	if (data.seconds_required >= 0 && (data.action == "ENDLEVEL")) data_sstr << "<seconds_required>" << data.seconds_required << "</seconds_required>";
	data_sstr << "</event>";
	return data_sstr.str();
}

string GameEvents::format_event_data_csv(GameData data)
{
	ostringstream data_sstr;

	data_sstr << "\"" << GameEvents::sessionid << "\", ";
	data_sstr << "\"" << data.timestamp << "\", ";
	data_sstr << "\"" << data.action << "\", ";
	data_sstr <<  "\"" << data.level << "\", ";
	data_sstr << "\"" << data.update << "\", ";
	data_sstr << "\"" << data.seconds << "\", ";
	data_sstr << "\"" << data.which_lix << "\", ";
	data_sstr << "\"" << data.lix_required << "\", ";
	data_sstr << "\"" << data.lix_saved << "\", ";
	data_sstr << "\"" << data.skills_used << "\", ";
	data_sstr << "\"" << data.seconds_required << "\" \n";
	return data_sstr.str();
}

string GameEvents::extract_sessionid(string response_string) {
	try
	{
		//Try to extract sessionid
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(response_string);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		string tmpsessionid = object->get("sessionid");

		ostringstream tmpmsg;
		tmpmsg << " ===== Got a sessionid: " << GameEvents::sessionid;
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

string GameEvents::extract_token(string response_string) {
	try
	{
		//Try to extract token
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(response_string);
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		string tmptoken = object->get("token");

		ostringstream tmpmsg;
		tmpmsg << " ===== Got a token: " << GameEvents::token;
		Log::log(Log::INFO, tmpmsg.str());

		return tmptoken;
	}
	catch (std::exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "Exception when trying to read token: " << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
		//GameEvents::token = "";
		return "";
	}


}
