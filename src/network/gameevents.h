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
#include <Poco/StreamCopier.h>

#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

#include <sstream>
#include <exception>
#include <ctime>


#include "../other/file/log.h"
#include "../gameplay/replay.h"


class GameEvents
{
    public:
        //GameEvents();
        //virtual ~GameEvents();
		static std::string format_replay_data(Replay::Data replay_data);
		static std::string format_event(std::string event);
        static bool send_event(std::string event, signed int number_of_attempts);
        //static bool send_event(Replay::Data replay_data, signed int number_of_attempts);


    protected:
    private:
        static std::string service_endpoint;
        static std::string clientid;
        static std::string apikey;
        static std::string token;
        static std::string sessionid;
        static signed int max_number_attempts;
        static bool connection_is_setup;

        static void configure();
        static std::string get_token();
        static Poco::Net::HTTPResponse::HTTPStatus do_request(std::string endpoint,
        		std::string resource, std::string method, std::string request_body,
				Poco::Net::HTTPResponse& response, std::ostringstream& output_stream);
        static bool send_event_attempt(std::string event);
};
