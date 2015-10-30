#include "gameevents.h"



using namespace std;

string GameEvents::service_endpoint = "";
string GameEvents::clientid = "";
string GameEvents::apikey = "";
string GameEvents::token = "";
string GameEvents::sessionid = "";
bool GameEvents::connection_is_setup = false;
signed int GameEvents::max_number_attempts = 3;

//////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////

string GameEvents::format_event(string event)
{

	//prepare request body
	time_t timestamp = time(0);
	ostringstream request_body_ss;
	request_body_ss << "<xml>";
	request_body_ss << "<timestamp>" << timestamp << "</timestamp>";
	request_body_ss << "<event>" << event << "</event>";
	request_body_ss << "</xml>";

	string output;
	output = request_body_ss.str();
    return(output);

}

string GameEvents::format_replay_data(Replay::Data data)
{
	return "hello world";
}

bool GameEvents::send_event(string event, signed int number_of_attempts) {

	bool success;
	success = false;

	signed int counter;
	counter = 1;

	ostringstream tmpmsg;
	tmpmsg << "Sending event: ' " << event << "'";
	Log::log(Log::INFO, tmpmsg.str());

	try {
		while ((counter <= number_of_attempts) and (not success))
		{
			ostringstream tmpmsg;
			tmpmsg << "Attempt number " << counter;
			Log::log(Log::INFO, tmpmsg.str());
			success = GameEvents::send_event_attempt(event);
			counter++;
		}
	}
	catch (std::exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "...Unfortunately I received an unexpected exception:" << ex.what();
		Log::log(Log::ERROR, tmpmsg.str());
		return(false);
	}
}

//bool GameEvents::send_event(Replay::Data& replay_data, signed int number_of_attempts) {
//
//	bool success;
//	success = false;
//
//	signed int counter;
//	counter = 1;
//	Replay::Vec data = replay_data.get_data();
//
//	ostringstream replay_data_sstr;
//
//	for (Replay::It itr = replay_data.begin(); itr != replay_data.end(); ++itr) {
//	        std::string word
//	         = itr->action == Replay::SPAWNINT     ? gloB->replay_spawnint
//	         : itr->action == Replay::NUKE         ? gloB->replay_nuke
//	         : itr->action == Replay::ASSIGN       ? gloB->replay_assign_any
//	         : itr->action == Replay::ASSIGN_LEFT  ? gloB->replay_assign_left
//	         : itr->action == Replay::ASSIGN_RIGHT ? gloB->replay_assign_right
//	                                               : Language::common_cancel;
//	        if (itr->action == ASSIGN || itr->action == ASSIGN_LEFT
//	                                  || itr->action == ASSIGN_RIGHT) {
//	            word += "=";
//	            word += LixEn::ac_to_string(static_cast <LixEn::Ac> (itr->skill));
//	        }
//	        replay_data_sstr << IO::LineBang(itr->update, itr->player, word, itr->what);
//	    }
//
//	ostringstream tmpmsg;
//	tmpmsg << "Sending event: ' " << replay_data_sstr.str() << "'";
//	Log::log(Log::INFO, tmpmsg.str());
//
//	try {
//		while ((counter <= number_of_attempts) and (not success))
//		{
//			ostringstream tmpmsg;
//			tmpmsg << "Attempt number " << counter;
//			Log::log(Log::INFO, tmpmsg.str());
//			success = GameEvents::send_event_attempt(event);
//			counter++;
//		}
//	}
//	catch (std::exception &ex)
//	{
//		ostringstream tmpmsg;
//		tmpmsg << "...Unfortunately I received an unexpected exception:" << ex.what();
//		Log::log(Log::ERROR, tmpmsg.str());
//		return(false);
//	}
//}

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


            try {

            	signed int tmp5;
            	tmp5 = cfg.lookup("service.max_number_attempts");
            	//string tmp5_str = tmp5.str();
				//signed int tmp5_int = std::atoi(tmp5_str.c_str());
				if ( (tmp5 > 0) && (tmp5 < 100)) {
					GameEvents::max_number_attempts = tmp5;
				}
            }
			catch (std::exception &ex) {
				ostringstream tmpmsg;
				tmpmsg << "Could not read max_number_attempts, using default value 3. Exception: " << ex.what();
				Log::log(Log::INFO, tmpmsg.str());
			}

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
    //try to connect to the service
    try
    {
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

        		// Prepare response object
        		Poco::Net::HTTPResponse response;

        		// Prepare response stream
        		ostringstream response_stream;

        		Poco::Net::HTTPResponse::HTTPStatus response_status;
        		response_status = GameEvents::do_request(service_endpoint, resource,
        				Poco::Net::HTTPRequest::HTTP_POST, request_body, response, response_stream);

        		if (response_status == Poco::Net::HTTPResponse::HTTP_OK ) {
        			//response: OK
        			ostringstream tmpmsg;
        			tmpmsg << "Successfully sent token request, status " << response_status;
        			Log::log(Log::INFO, tmpmsg.str());

        			std::string response_body;
        			response_body = response_stream.str();

        			try
        			{
        				//Try to extract token
        				Poco::JSON::Parser parser;
        				Poco::Dynamic::Var result = parser.parse(response_body);
        				Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        				string tmptoken = object->get("token");
        				GameEvents::token = tmptoken;
        				ostringstream tmpmsg;
        				tmpmsg << " ===== Got a token: " << GameEvents::token;
        				Log::log(Log::INFO, tmpmsg.str());
        			}
        			catch (Poco::Exception &ex)
        			{
        				ostringstream tmpmsg;
        				tmpmsg << "Exception when trying to read token: " << ex.displayText();
        				Log::log(Log::ERROR, tmpmsg.str());
        				GameEvents::token = "";
        			}
        		}
        		else
        		{
        			//response failed
        			ostringstream tmpmsg;
        			tmpmsg << "Failed to get authentication token. Reason: " <<  response.getReason();
        			Log::log(Log::INFO, tmpmsg.str());
        			GameEvents::token = "";
        		}
        	}
        	else
        	{
        		Log::log(Log::INFO, "I already have a token. Continuing, so that I return the existing one...");
        	}
        }
        else
        {
        	Log::log(Log::INFO, "Could not set up the connection settings.");
        }
    }
    catch (Poco::Exception& e) {
        ostringstream tmpmsg;
        tmpmsg << "Exception: " << e.what() << ". Message: " << e.message() ;
        Log::log(Log::ERROR, tmpmsg.str());
    }

    return GameEvents::token;
}


Poco::Net::HTTPResponse::HTTPStatus GameEvents::do_request(string endpoint,
		string resource, string method, string request_body,
		Poco::Net::HTTPResponse& response, ostringstream& output_stream)
{
	// prepare session
	string request_url = "";
	request_url = endpoint + "/" + resource;
	Poco::URI request_uri(request_url);
	Poco::Net::HTTPClientSession client_session(request_uri.getHost(), request_uri.getPort());

	// prepare path
	string request_path(request_uri.getPathAndQuery());
	if (request_path.empty()) request_path = "/";

	//In Windows we need to initialize the network
	//TODO: Do I need a check here to avoid problems in other OS's?
	Poco::Net::initializeNetwork();

	// send request
	Poco::Net::HTTPRequest request(method, request_path);


	//is it a POST query and a non-empty request body?
	if (method==Poco::Net::HTTPRequest::HTTP_POST)
	{
		if  (not (request_body.empty() || request_body.size()==0))
		{
			request.setContentType("application/json");
			request.setKeepAlive(true);
			request.setContentLength( request_body.length() );
			Log::log(Log::INFO, "Sending request...");
			client_session.sendRequest(request) << request_body;

		}
		else {
			//throw badly formatted request exception
			throw Poco::Exception("Bad request");
			client_session.reset();
		}
	}
	else {
		//non-POST query does not need a request body
		client_session.sendRequest(request);
	}

	std::istream &is = client_session.receiveResponse(response);
	Poco::StreamCopier::copyStream(is, output_stream);

	Poco::Net::HTTPResponse::HTTPStatus status = response.getStatus();
	ostringstream tmpmsg;
	tmpmsg << " do_request(): Received response: " << status;
	Log::log(Log::INFO, tmpmsg.str());

	return(status);
}


bool GameEvents::send_event_attempt(string event)
{
	//TODO: if token is refused, try to get a new one and try once again

	string resource = "commitevent";
	string current_token;
	try {
		Log::log(Log::INFO, "Send event attempt, first making sure I have a token.");
		current_token = GameEvents::get_token();
		ostringstream tmpmsg;
		tmpmsg << "My current token is " << current_token;
		Log::log(Log::INFO,  tmpmsg.str());
	}
	catch (std::exception &ex)
	{
		ostringstream tmpmsg;
		tmpmsg << "Unexpected exception while trying to get token: " << ex.what();
		Log::log(Log::ERROR,  tmpmsg.str());
		return(false);
	}

    if (connection_is_setup == false) {
        Log::log(Log::ERROR, "Could not set up the connection.");
        return(false);
    }
    else {
        if (current_token.empty() || current_token.size()==0) {
            Log::log(Log::ERROR, "Could not get a token.");
            return(false);
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

        	try
        	{
        		Poco::Net::HTTPResponse::HTTPStatus response_status;

        		Poco::Net::HTTPResponse response;
        		ostringstream response_stream;

        		response_status = GameEvents::do_request(GameEvents::service_endpoint,
        				resource, Poco::Net::HTTPRequest::HTTP_POST, request_body,
						response, response_stream);

        		if (response_status == Poco::Net::HTTPResponse::HTTP_OK ||
        				response_status == Poco::Net::HTTPResponse::HTTP_CREATED ) {
        			//response: OK
        			ostringstream tmpmsg;
        			tmpmsg << "Successfully sent game event, status " << response_status;
        			Log::log(Log::INFO, tmpmsg.str());

        			std::string response_body;
        			response_body = response_stream.str();

        			try
        			{
        				//Try to extract returned message
        				Poco::JSON::Parser parser;
        				Poco::Dynamic::Var result = parser.parse(response_body);
        				Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        				ostringstream tmpmsg;
        				string msg = object->get("message");
        				tmpmsg << "Returned message: " << msg;
        				Log::log(Log::INFO,  tmpmsg.str());
        				return(true);
        			}
        			catch (Poco::Exception &ex)
        			{
        				ostringstream tmpmsg;
        				tmpmsg << "Exception when trying to read message: " << ex.displayText();
        				Log::log(Log::ERROR,  tmpmsg.str() );
        				return(false);
        			}

        		}
        		else if (response_status == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
        			Log::log(Log::INFO, "Token unauthorized. Resetting the token variable.");
        			GameEvents::token = "";
        			return(false);
        		}
        		else {
        			//response failed
        			ostringstream tmpmsg;
        			tmpmsg << "Failed to commit game event. Reason: " << response.getReason();
        			Log::log(Log::INFO,  tmpmsg.str() );
        			return(false);
        		}

        	}
        	catch (Poco::Exception &ex)
			{
        		ostringstream tmpmsg;
        		tmpmsg << "Received unexpected exception: " << ex.displayText();
            	Log::log(Log::ERROR, tmpmsg.str());
            	ex.rethrow();
            	//return(false);
			}

        }
    }
}


