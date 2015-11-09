#include "gameevents.h"

//#include "../other/myalleg.h" //needs to come first

#include <algorithm>  // for copy
#include <iterator>
#include <string>
#include <iostream>   // for cout, istream
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <exception>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>

//#include "../other/user.h"
//#include "../other/verify.h"
#include "../other/globals.h"
#include "../other/file/log.h"
#include "../other/file/filename.h"
#include "../other/language.h"

#include "../lix/lix_enum.h" // initialize strings
//#include "../graphic/png/loadpng.h"
#include "../gameplay/replay.h"

using namespace std;

string GameEvents::service_endpoint = "";
string GameEvents::clientid = "";
string GameEvents::apikey = "";
string GameEvents::token = "";
string GameEvents::sessionid = "";
bool GameEvents::connection_is_setup = false;
bool GameEvents::offline_mode = false;
bool GameEvents::record_local_file = true;
signed int GameEvents::max_number_attempts = 3;

//////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////

GameEvents::Data::Data() {
	//Constructor, set the timestamp

	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];
	ostringstream timestampstream;
	time ( &rawtime );
	timeinfo = gmtime ( &rawtime );
	strftime(buffer,80,"%Y-%m-%dT%H:%M:%S",timeinfo);

	timestampstream << buffer;

	std::string timestampstring = timestampstream.str();
	timestampstring += "Z"; //Add UTC as timezone, since we used gmttime

	this->timestamp = timestampstring;
	this->action="";
	this->level ="";
	this->which_lix=-1;
	this->update=-1;
	this->seconds=-1;
	this->lix_required=0;
	this->lix_saved=0;
	this->skills_used=0;
	this->seconds_required=0;
}

void GameEvents::Data::load_event_data(Replay::Data data, std::string level) {

	//GameEvent::Data event_data;

	//Convert info in replay_data object into words to send to service
	std::string action_word;
	action_word = data.action == Replay::SPAWNINT ? gloB->replay_spawnint
			: data.action == Replay::NUKE ? gloB->replay_nuke
					: data.action == Replay::ASSIGN ? gloB->replay_assign_any
							: data.action == Replay::ASSIGN_LEFT  ? gloB->replay_assign_left
									: data.action == Replay::ASSIGN_RIGHT ? gloB->replay_assign_right
											: Language::common_cancel;

	if (data.action == Replay::ASSIGN || data.action == Replay::ASSIGN_LEFT
			|| data.action == Replay::ASSIGN_RIGHT) {
		action_word += "=";
		action_word += LixEn::ac_to_string(static_cast <LixEn::Ac> (data.skill));
	}

	this->action = action_word;
	this->level = level;
	this->update = data.update;
	this->which_lix = data.what;

	//Convert update to seconds
	signed long secs = this->update / gloB->updates_per_second;
	this->seconds = secs;
}

void GameEvents::Data::prepare_event_data(std::string action_word, signed long update, std::string level) {
	this->action = action_word;
	this->level = level;
	this->update = update;

	//Convert update to seconds
	signed long secs = this->update / gloB->updates_per_second;
	this->seconds = secs;
}

void GameEvents::Data::load_result_data(Result result, Level level) {

	this->action = "ENDLEVEL";
	this->update = -1;
	this->which_lix = -1;

	this->lix_required = level.required;
	this->lix_saved = result.lix_saved;
	this->skills_used = result.skills_used;
	this->seconds_required=level.seconds;
	this->level=level.level_filename;


	//Convert update to seconds
	this->seconds = this->update;
}


void GameEvents::send_event(GameEvents::Data event_data) {

	if (not GameEvents::connection_is_setup) {
		Log::log(Log::INFO, "Connection is not yet set up. Running 'configure()'...");
		GameEvents::configure();
		if (not GameEvents::max_number_attempts || GameEvents::max_number_attempts < 0 || GameEvents::max_number_attempts > 100) {
			Log::log(Log::ERROR, "Invalid value for max_number_attempts variable. Using '1' as default.");
			GameEvents::max_number_attempts = 1;
		}
	}

	if (GameEvents::offline_mode==false) {

		GameEvents::send_event(event_data, GameEvents::max_number_attempts);
	}
	else {
		//Log::log(Log::INFO, "Running in offline mode.");
	}


	if (GameEvents::offline_mode || GameEvents::record_local_file) {
		//Log::log(Log::INFO, "Logging event offline.");
		GameEvents::log_event_locally(event_data);
	}
}

void GameEvents::send_event(GameEvents::Data data, signed int number_of_attempts) {

	bool success;
	success = false;

	signed int counter;
	counter = 1;

	//Format data
	std::string event;
	event = GameEvents::format_event_data(data);

	//ostringstream tmpmsg;
	//tmpmsg << "Sending event...'" << event.substr(0,70);
	//Log::log(Log::INFO, tmpmsg.str());

	if (not GameEvents::offline_mode) {
		while ((counter <= number_of_attempts) and (not success) and (not GameEvents::offline_mode))
		{
			//ostringstream tmpmsg;
			//tmpmsg << "Attempt number " << counter;
			//Log::log(Log::INFO, tmpmsg.str());
			try {
				GameEvents::send_event_attempt(event);
				success = true;
			}
			catch (Poco::Net::NotAuthenticatedException& ex) {
				ostringstream tmpmsg;
				tmpmsg << "POCO not authenticated:" << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				counter++;
			}
			catch (Poco::Net::ConnectionRefusedException& ex) {
				ostringstream tmpmsg;
				tmpmsg << "POCO connection refused: " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				GameEvents::offline_mode = true;
				break;
			}
			catch (Poco::Net::NetException& ex) {
				ostringstream tmpmsg;
				tmpmsg << "POCO Net exception: " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				break;
			}
			catch (Poco::Exception& ex) {
				ostringstream tmpmsg;
				tmpmsg << "POCO exception: " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				break;
			}
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Unexpected exception: " << ex.what();
				Log::log(Log::ERROR, tmpmsg.str());
				break;
			}
		}
	}
	else {
		Log::log(Log::INFO, "Working in offline mode.");
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
            cfg.readFile("config.cfg");
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

        //try to read the service endpoint

        try
        {
            string tmp = cfg.lookup("service.endpoint");
            GameEvents::service_endpoint = tmp;
            if (GameEvents::service_endpoint.empty() || GameEvents::service_endpoint.size()==0) {
                Log::log(Log::ERROR, "Could not find configuration for service endpoint.");
            }
            else {
                //Make sure there's no trailing space in the service_endpoint string
                if ( GameEvents::service_endpoint[service_endpoint.size()-1] == '/' )
                    GameEvents::service_endpoint = GameEvents::service_endpoint.substr(0, GameEvents::service_endpoint.size()-1);
            }


            string tmp2 = cfg.lookup("auth.clientid");
            GameEvents::clientid = tmp2;

            string tmp3 = cfg.lookup("auth.apikey");
            GameEvents::apikey = tmp3;

            string tmp4 = cfg.lookup("auth.sessionid");
            GameEvents::sessionid = tmp4;

            bool tmp5 = cfg.lookup("record_local_file"); //Should i record a local file as well?
            GameEvents::record_local_file = tmp5;


            try {
            	signed int tmp6;
            	tmp6 = cfg.lookup("service.max_number_attempts");
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


    		string request_url = GameEvents::service_endpoint + "/" + resource;

    		Poco::Net::HTTPResponse response;
    		ostringstream response_stream;
    		Poco::Net::HTTPResponse::HTTPStatus response_status;
    		response_status = GameEvents::post(request_url, request_body, response_stream);


    		if (response_status == Poco::Net::HTTPResponse::HTTP_OK ) {
    			//response: OK
    			//ostringstream tmpmsg;
    			//tmpmsg << "Successfully sent token request, status " << response_status;
    			//Log::log(Log::INFO, tmpmsg.str());

    			try
    			{
    				//Try to extract token
    				Poco::JSON::Parser parser;
    				Poco::Dynamic::Var result = parser.parse(response_stream.str());
    				Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    				string tmptoken = object->get("token");
    				GameEvents::token = tmptoken;
    				//ostringstream tmpmsg;
    				//tmpmsg << " ===== Got a token: " << GameEvents::token;
    				//Log::log(Log::INFO, tmpmsg.str());
    			}
    			catch (Poco::Exception &ex)
    			{
    				ostringstream tmpmsg;
    				tmpmsg << "Exception when trying to read token: " << ex.displayText();
    				Log::log(Log::ERROR, tmpmsg.str());
    				GameEvents::token = "";
    			}
    		}
    		else if (response_status == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
    			Log::log(Log::INFO, "Token unauthorized. Resetting the token variable.");
    			GameEvents::token = "";
    			throw Poco::Net::NotAuthenticatedException("Token unauthorized");
    		}
    		else {
    			//response failed
    			ostringstream tmpmsg;
    			tmpmsg << "Failed to commit game event. Reason: " << response.getReason();
    			Log::log(Log::INFO,  tmpmsg.str() );
    			throw Poco::Net::NetException(response.getReason());
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
	string resource = "commitevent";
	string current_token;
	try {
		//Log::log(Log::INFO, "Send event attempt, first making sure I have a token.");
		current_token = GameEvents::get_token();
		//ostringstream tmpmsg;
		//tmpmsg << "My current token is " << current_token;
		//Log::log(Log::INFO,  tmpmsg.str());
	}
	catch (Poco::Net::ConnectionRefusedException& ex) {
		ostringstream tmpmsg;
		tmpmsg << "POCO connection refused: " << ex.what() << ". Setting offline mode. ";
		Log::log(Log::ERROR, tmpmsg.str());
		GameEvents::offline_mode = true;
	}
	catch (std::exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "Unexpected exception while trying to get token: " << ex.what();
		Log::log(Log::ERROR,  tmpmsg.str());
		throw Poco::Net::NotAuthenticatedException("Could not get a token.");
	}

	if (connection_is_setup == false) {
		Log::log(Log::ERROR, "Could not set up the connection.");

	}
	else {
		if (current_token.empty() || current_token.size()==0) {
			Log::log(Log::ERROR, "Could not get a token.");
			throw Poco::Net::NotAuthenticatedException("Could not get a token.");
		}
		else
		{
			//OK, I have a non-empty token

			//prepare request body
			time_t timestamp = time(0);
			ostringstream request_body_ss;
			request_body_ss << "{\"token\" : \"" << current_token << "\", ";
			request_body_ss << "\"timestamp\" : \"" << timestamp << "\", ";
			request_body_ss << "\"gameevent\" : \"" << event << "\"";
			request_body_ss << "}";

			string request_body;
			request_body = request_body_ss.str();

			string request_url = "";
			request_url = GameEvents::service_endpoint + "/" + resource;

			Poco::Net::HTTPResponse response;
			ostringstream response_stream;
			Poco::Net::HTTPResponse::HTTPStatus response_status;
			response_status = GameEvents::post(request_url, request_body, response_stream);



			if (response_status == Poco::Net::HTTPResponse::HTTP_CREATED ) {
				//response: OK
				//ostringstream tmpmsg;
				//tmpmsg << "Successfully sent game event, status " << response_status;
				//Log::log(Log::INFO, tmpmsg.str());

			}
			else if (response_status == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
				Log::log(Log::ERROR, "Token unauthorized. Resetting the token variable.");
				GameEvents::token = "";
				throw Poco::Net::NotAuthenticatedException("Token expired");
			}
			else {
				//response failed
				ostringstream tmpmsg;
				tmpmsg << "Failed to commit game event. Reason: " << response.getReason();
				Log::log(Log::ERROR,  tmpmsg.str() );
				throw Poco::Net::NetException(response.getReason());
			}

		}
	}
}


Poco::Net::HTTPResponse::HTTPStatus GameEvents::post(string url, string payload, ostringstream& output_stream) {
	// prepare session
	Poco::URI request_uri(url);
	Poco::Net::HTTPClientSession client_session(request_uri.getHost(), request_uri.getPort());

	// prepare path
	string request_path(request_uri.getPathAndQuery());
	if (request_path.empty()) request_path = "/";

	//In Windows we need to initialize the network
	//TODO: Do I need a check here to avoid problems in other OS's?
	Poco::Net::initializeNetwork();

	// prepare request
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, request_path);
	if  (not (payload.empty() || payload.size()==0))
	{
		request.setContentType("application/json");
		request.setKeepAlive(true);
		request.setContentLength( payload.length() );
		//Log::log(Log::INFO, "Sending request...");

		//send request
		client_session.sendRequest(request) << payload;

		Poco::Net::HTTPResponse response;

		std::istream &is = client_session.receiveResponse(response);
		Poco::StreamCopier::copyStream(is, output_stream);

		//response.getStatus();
		return response.getStatus();
	}
	else {
		//throw badly formatted request exception
		client_session.reset();
		throw Poco::Exception("Bad request.");
	}

}

bool GameEvents::file_exists(std::string filename) {
    ifstream f(filename.c_str(), ifstream::in);
    return !f.fail(); // using good() could fail on empty files
}

void GameEvents::log_event_locally(GameEvents::Data event_data) {

	string filename = "local_log.csv";
	string formatted_line = GameEvents::format_event_data_csv(event_data);
	ofstream myfile;
	try {
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





string GameEvents::format_event_data(GameEvents::Data data)
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

string GameEvents::format_event_data_csv(GameEvents::Data data)
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
