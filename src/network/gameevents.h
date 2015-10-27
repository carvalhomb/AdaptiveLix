#pragma once

//#include "../other/myalleg.h" //needs to come first

#include "../other/lmain.h" // Main object to manage the different parts of the program
#include "../other/user.h"
#include "../other/verify.h"
#include "../other/file/log.h"
#include "../other/language.h"

#include "../lix/lix_enum.h" // initialize strings
#include "../graphic/png/loadpng.h"



#include "../other/user.h"
#include "../other/verify.h"
#include "../other/globals.h"

#include <algorithm>  // for copy
#include <iterator>
#include <string>
#include <iostream>   // for cout, istream

#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>


#include "../other/file/log.h"


class GameEvents
{
    public:
        //GameEvents();
        //virtual ~GameEvents();

        static bool send_event(std::string event);
        static bool close_connection();
        static int mymain();

    protected:
    private:
        static void configure();
        static std::string get_token();
        static std::string service_endpoint;
        static std::string clientid;
        static std::string apikey;
        static std::string token;
        static bool connection_is_setup;

};
