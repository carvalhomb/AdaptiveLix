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
    Poco::URI uri("http://google.com");
    Poco::Net::HTTPClientSession client_session(uri.getHost(), uri.getPort());

    // Prepare and send request
    std::string path(uri.getPathAndQuery());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
        path, Poco::Net::HTTPMessage::HTTP_1_1);
    client_session.sendRequest(req);

    // Get response
    Poco::Net::HTTPResponse res;
    std::istream& is = client_session.receiveResponse(res);

    // Print to standard output
    std::copy(std::istream_iterator<char>(is),
        std::istream_iterator<char>(),
        std::ostream_iterator<char>(std::cout));
    return 0;
  } catch (Poco::Exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
}

int main()
{
    int myresponse;

    std::cout<<"Hello world !"<<std::endl;
    GameEvents * ge = new GameEvents();
    std::cout<<"Trying to connect to http://google.com ... "<<std::endl;
    myresponse = ge->test_connection();
    std::cout << "Completed communication, returned " << myresponse << "."<<std::endl;
    delete ge;
    return myresponse;
    return 0;
}

