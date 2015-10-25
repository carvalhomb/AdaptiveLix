#include "gameevents.h"

#include <sstream>

using namespace std;
//using namespace libconfig;

string GameEvents::service_endpoint = "";
string GameEvents::clientid = "";
string GameEvents::apikey = "";
string GameEvents::token = "";

bool GameEvents::configure()
{
    libconfig::Config cfg;



    //Try to open the config file
    try {
        cfg.readFile("config.cfg");
    }
    catch(const libconfig::FileIOException &fioex)
    {
        Log::log(Log::ERROR, "I/O error while reading file.");
        return false;
    }
    catch(const libconfig::ParseException &pex)
    {
        //string msg = "Parse error at " << string(pex.getFile()) << ":" << pex.getLine() << " - " << pex.getError();
        ostringstream msg;
        msg << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
        Log::log(Log::ERROR, msg.str());
        return false;
    }

    //try to read the service endpoint

    try
    {

        string tmp = cfg.lookup("service.endpoint");
        GameEvents::service_endpoint = tmp;

        string tmp2 = cfg.lookup("auth.clientid");
        GameEvents::clientid = tmp2;

        string tmp3 = cfg.lookup("auth.apikey");
        GameEvents::apikey = tmp3;

        return true;
    }
    catch(const libconfig::SettingNotFoundException &nfex)
    {
        Log::log(Log::ERROR, "Required settings not found.");
        return false;
    }



}

int GameEvents::init_connection()
{
    const string handshake_method = "token";

    //try to connect to the service
    try
    {
        // Initialize session
        if (service_endpoint.empty() || service_endpoint.size()==0) {

            Log::log(Log::ERROR, "Could not find configuration for service endpoint.");
            return(EXIT_FAILURE);

        } else {

            //Make sure there's no trailing space in the service_endpoint string
            if ( service_endpoint[service_endpoint.size()-1] == '/' )
                service_endpoint = service_endpoint.substr(0, service_endpoint.size()-1);

            string url = "";
            url = service_endpoint + "/" + handshake_method;

            Log::log(Log::INFO, "Trying to connect to: " + url);
            Poco::URI uri(url);
            Poco::Net::HTTPClientSession client_session(uri.getHost(), uri.getPort());

            // Prepare and send request
            string path(uri.getPathAndQuery());

            //In Windows we need to initialize the network
            Poco::Net::initializeNetwork();

            // Get response
            Poco::Net::HTTPResponse res;

            //Make a post request
            Poco::Net::HTTPRequest req2(Poco::Net::HTTPRequest::HTTP_POST, path);
            req2.setContentType("application/json");
            req2.setKeepAlive(true);
            string reqBody("{\"clientid\":\""+ GameEvents::clientid +"\",\"apikey\":\""+ GameEvents::apikey +"\"}");
            req2.setContentLength( reqBody.length() );

            //Send request
            client_session.sendRequest(req2) << reqBody;

            //Receive response
            istream& is = client_session.receiveResponse(res);

            // Print to standard output
            ostringstream response_body;
            Log::log(Log::INFO, "Body of the request:");
            Log::log(Log::INFO, reqBody);

            response_body << is.rdbuf();

            //copy(istream_iterator<char>(is),
            //istream_iterator<char>(),
            //ostream_iterator<char>(response_body) );
            Log::log(Log::INFO, "Body of the response:");
            Log::log(Log::INFO, response_body.str());

            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(response_body.str());
            Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
            string msg = object->get("token");
            Log::log(Log::INFO, msg);
            //ostream_iterator<char>(cout));
            return(EXIT_SUCCESS);
        }
    }
    catch (Poco::Exception& e) {
        ostringstream msg;
        msg << "Exception: " << e.what() << ". Message: " << e.message() ;
        Log::log(Log::ERROR, msg.str());
        return(EXIT_FAILURE);
    }
}


int GameEvents::send_event()
{
    return(EXIT_SUCCESS);
}

int GameEvents::close_connection()
{
    return(EXIT_SUCCESS);
}

int GameEvents::mymain()
{
    int response;

    Log::log(Log::INFO, "Reading configuration file... ");

    bool success_config = GameEvents::configure();

    if (success_config) {
        Log::log(Log::INFO,"Starting connection... ");
        response = GameEvents::init_connection();
        ostringstream msg;
        msg << "Completed communication, returned " << response << ".";
        Log::log(Log::INFO, msg.str());

    } else {
        return(EXIT_FAILURE);
    }

    //delete ge;
    //Log::deinitialize();
    return response;
}

