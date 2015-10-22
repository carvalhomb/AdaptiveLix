#include "gameevents.h"

//using namespace std;
//using namespace libconfig;


GameEvents::GameEvents()
{
    //ctor
}

GameEvents::~GameEvents()
{
    //dtor
}


int GameEvents::test_connection()
{
    libconfig::Config cfg;

    std::string token_method_setting;
    std::string endpoint_setting;

    std::string url;
    std::string endpoint;
    std::string token_method;

    token_method_setting = "token_method";
    endpoint_setting = "endpoint";

    //Try to open the config file
    try {
        cfg.readFile("config.cfg");
    }
    catch(const libconfig::FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return(EXIT_FAILURE);
    }
    catch(const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return(EXIT_FAILURE);
    }

    //try to read the service endpoint
    url = "";
    try
    {
        std::string endpoint=cfg.lookup(endpoint_setting);
        std::string token_method=cfg.lookup(token_method_setting);
        std::cout << "endpoint: " << endpoint << std::endl;
        std::cout << "token_method: " << token_method << std::endl;
        //put together the url to request the token
        url = endpoint + "/" + token_method;

    }
    catch(const libconfig::SettingNotFoundException &nfex)
    {
        std::cout <<  "Required settings not found." << std::endl;
        return(EXIT_FAILURE);
    }


    //try to connect to the service
    try
    {
        // Initialize session

        std::cout << "Trying to connect to: " << url << std::endl;
        Poco::URI uri(url);
        Poco::Net::HTTPClientSession client_session(uri.getHost(), uri.getPort());

        // Prepare and send request
        std::string path(uri.getPathAndQuery());

        //In Windows we need to initialize the network
        Poco::Net::initializeNetwork();

        // Get response
        Poco::Net::HTTPResponse res;

        //Make a post request
        Poco::Net::HTTPRequest req2(Poco::Net::HTTPRequest::HTTP_POST, path);
        req2.setContentType("application/json");
        req2.setKeepAlive(true);
        std::string reqBody("{\"clientid\":\"myclientid\",\"apikey\":\"myapikey\"}");
        req2.setContentLength( reqBody.length() );

        //Send request
        client_session.sendRequest(req2) << reqBody;

        //Receive response
        std::istream& is = client_session.receiveResponse(res);

        // Print to standard output
        std::cout << "Body of the request and the response:" << std::endl;
        std::cerr << reqBody << std::endl;
        std::copy(std::istream_iterator<char>(is),
        std::istream_iterator<char>(),
        std::ostream_iterator<char>(std::cout));
        return 0;
    }
    catch (Poco::Exception& e) {
        std::cerr << "Exception: " << e.what() << ". " << e.message() << std::endl;
        return 1;
    }
}

int main()
{
    int myresponse;

    std::cout<<"Hello world !"<<std::endl;
    GameEvents * ge = new GameEvents();
    std::cout<<"Starting connection... "<<std::endl;
    myresponse = ge->test_connection();
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Completed communication, returned " << myresponse << "."<<std::endl;
    delete ge;
    return myresponse;
}

