#include <algorithm>  // for copy
#include <iterator>
#include <string>
#include <iostream>   // for cout, istream

#include <Poco/Exception.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

class GameEvents
{
    public:
        GameEvents();
        virtual ~GameEvents();
        int test_connection();
    protected:
    private:

};
