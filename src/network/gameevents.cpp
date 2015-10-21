#include "gameevents.h"




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
    try {
    // Initialize session
    Poco::URI uri("http://127.0.0.1:5000/gameevents/api/v1.0/token");
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
  } catch (Poco::Exception& e) {
    std::cerr << "Exception: " << e.what() << ". " << e.message() << std::endl;
    return 1;
  }
}

int main()
{
    int myresponse;

    std::cout<<"Hello world !"<<std::endl;
    GameEvents * ge = new GameEvents();
    std::cout<<"Trying to connect to http://127.0.0.1:5000 ... "<<std::endl;
    myresponse = ge->test_connection();
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Completed communication, returned " << myresponse << "."<<std::endl;
    delete ge;
    return myresponse;
}

