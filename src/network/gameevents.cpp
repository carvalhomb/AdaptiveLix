#include "gameevents.h"

#include <sstream>
#include <ctime>

using namespace std;
//using namespace libconfig;

string GameEvents::service_endpoint = "";
string GameEvents::clientid = "";
string GameEvents::apikey = "";
string GameEvents::token = "";
string GameEvents::sessionid = "";
bool GameEvents::connection_is_setup = false;

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

            connection_is_setup = true;
        }
        catch(const libconfig::SettingNotFoundException &nfex)
        {
            Log::log(Log::ERROR, "Required settings not found.");
        }
    } else {} //Connection is already set up, skipping...

}

string GameEvents::get_token()
{

    //TODO: send a session id when creating the token so that the service can associate
    // the game events to the correct session id. I need to decide the format of this
    // session id. get from UP service?

    const string method = "token";
    //try to connect to the service
    try
    {
        //Is the connection set up? If not, run configure()
        if (connection_is_setup == false) {
            Log::log(Log::ERROR, "Connection is not set up. Running 'configure()'...");
            GameEvents::configure();
        }

        //And now, is the connection set up? And is the token empty?
        if (connection_is_setup == true && (GameEvents::token.empty() || GameEvents::token.size()==0) ) {

            // Call the service to get a token
            string url = "";
            url = service_endpoint + "/" + method;

            Log::log(Log::INFO, "Trying to connect to: " + url);
            Poco::URI uri(url);
            Poco::Net::HTTPClientSession client_session(uri.getHost(), uri.getPort());

            // Prepare and send request
            string path(uri.getPathAndQuery());

            //In Windows we need to initialize the network
            Poco::Net::initializeNetwork();

            // Get response
            Poco::Net::HTTPResponse response;

            //Make a post request
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path);
            request.setContentType("application/json");
            request.setKeepAlive(true);

            //prepare request body
            ostringstream request_body_ss;
            request_body_ss << "{\"clientid\" : \"" << GameEvents::clientid << "\", ";
            request_body_ss << "\"apikey\" : \"" << GameEvents::apikey << "\", ";
            request_body_ss << "\"sessionid\" : \"" << GameEvents::sessionid << "\"";
            request_body_ss << "}";

            string request_body;
            request_body = request_body_ss.str();

            request.setContentLength( request_body.length() );

            //Send request
            client_session.sendRequest(request) << request_body;

            //Receive response
            istream& is = client_session.receiveResponse(response);

            // Print to standard output
            ostringstream response_body;
            Log::log(Log::INFO, "Sending request for a token...");
            //Log::log(Log::INFO, reqBody);

            response_body << is.rdbuf();

            Log::log(Log::INFO, "Received response.");
            //Log::log(Log::INFO, response_body.str());

            Poco::Net::HTTPResponse::HTTPStatus status;
            status = response.getStatus();

            if (status == Poco::Net::HTTPResponse::HTTP_OK) {
                //response: OK
                Poco::JSON::Parser parser;
                Poco::Dynamic::Var result = parser.parse(response_body.str());
                Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
                string tmptoken = object->get("token");
                GameEvents::token = tmptoken;
                //Log::log(Log::INFO, "Received token: " + GameEvents::token);
                Log::log(Log::INFO, "Received authentication token.");

            }
            else {
                //response failed
                string msg;
                msg = response.getReason();
                Log::log(Log::INFO, "Failed to get authentication token. Reason: " + msg);
            }
        }
    }
    catch (Poco::Exception& e) {
        ostringstream msg;
        msg << "Exception: " << e.what() << ". Message: " << e.message() ;
        Log::log(Log::ERROR, msg.str());
    }

    return GameEvents::token;
}




bool GameEvents::send_event(string event)
{
    string method = "commitevent";
    GameEvents::token = GameEvents::get_token();

    if (connection_is_setup == false) {
        Log::log(Log::ERROR, "Could not set up the connection.");
        return(false);
    }
    else {
        if (GameEvents::token.empty() || GameEvents::token.size()==0) {
            Log::log(Log::ERROR, "Could not get a token.");
            return(false);
        }
        else
        {
            try {
                // prepare session
                string request_url = "";
                request_url = GameEvents::service_endpoint + "/" + method;
                Poco::URI request_uri(request_url);
                Poco::Net::HTTPClientSession client_session(request_uri.getHost(), request_uri.getPort());

                // prepare path
                string request_path(request_uri.getPathAndQuery());
                if (request_path.empty()) request_path = "/";

                //In Windows we need to initialize the network
                Poco::Net::initializeNetwork();

                //prepare request body
                time_t timestamp = time(0);
                ostringstream request_body_ss;
                //ss << seconds;
                //std::string ts = ss.str();
                request_body_ss << "{\"token\" : \"" << GameEvents::token << "\", ";
                request_body_ss << "\"timestamp\" : \"" << timestamp << "\", ";
                request_body_ss << "\"gameevent\" : \"" << event << "\"";
                request_body_ss << "}";

                string request_body;
                request_body = request_body_ss.str();


                // send request
                Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, request_path);
                request.setContentType("application/json");
                request.setKeepAlive(true);
                request.setContentLength( request_body.length() );
                //client_session.sendRequest(request);
                Log::log(Log::INFO, "Sending request to commit a game event...");
                //Log::log(Log::INFO, "Request body:" + request_body);
                client_session.sendRequest(request) << request_body;

                // get response into a variable
                Poco::Net::HTTPResponse response;
                istream& is = client_session.receiveResponse(response);
                ostringstream response_body;
                response_body << is.rdbuf();
                string response_string;
                response_string = response_body.str();
                //cout << response.getStatus() << " " << response.getReason() << endl;

                Poco::Net::HTTPResponse::HTTPStatus status;
                status = response.getStatus();

                if (status == Poco::Net::HTTPResponse::HTTP_OK || status == Poco::Net::HTTPResponse::HTTP_CREATED ) {
                    //response: OK
                    /*Poco::JSON::Parser parser;
                    Poco::Dynamic::Var result = parser.parse(response_body.str());
                    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
                    string tmptoken = object->get("token");
                    GameEvents::token = tmptoken;
                    //Log::log(Log::INFO, "Received token: " + GameEvents::token);*/
                    Log::log(Log::INFO, "Successfully sent game event.");
                    return(true);
                }
                else {
                    //response failed
                    string msg;
                    msg = response.getReason();
                    Log::log(Log::INFO, "Failed to commit game event... Reason: " + msg);
                    return(false);
                }

            }
            catch (Poco::Exception &ex)
            {
                Log::log(Log::ERROR, "Exception: " + ex.displayText());
            }

        }
    }
}

bool GameEvents::close_connection()
{
    return(true);
}

int GameEvents::mymain()
{
    bool response;

    Log::log(Log::INFO, "Reading configuration file... ");

    GameEvents::configure();

    Log::log(Log::INFO,"Starting connection... ");
    //string tokentmp = GameEvents::get_token();
    string event = "<xml>test</xml>";
    response = GameEvents::send_event(event);

    string return_status;
    if (response) {
        return_status = "successful";
    } else {
        return_status = "failed";
    }

    ostringstream msg;
    msg << "Program finished. Return status: " << return_status << ".";
    Log::log(Log::INFO, msg.str());

    return(0);
}

