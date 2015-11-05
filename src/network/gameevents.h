/*
 * Gameevents manage the formatting and sending of the
 * important game events to the external REST service
 * listening.
 */

#pragma once

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>
#include <Poco/Net/HTTPResponse.h>
#include "../gameplay/replay.h"
#include "../level/level.h"
#include "../other/user.h"


class GameEvents
{
    public:
		struct Data {
			std::string level;
			std::string action;
			signed long update;
			signed long seconds;
			signed long which_lix;
			time_t timestamp;
			int lix_required;
			int lix_saved;
			int skills_used;
			int seconds_required;

			Data();
			//virtual ~Data();
			void load_event_data(Replay::Data data, std::string level);
			void load_result_data(Result result, Level level);
		};


        //GameEvents();
        //virtual ~GameEvents();
		static std::string format_event_data(GameEvents::Data event_data);
        static void send_event(GameEvents::Data data);
        static void send_event(GameEvents::Data data, signed int number_of_attempts);



    protected:
    private:
        static std::string service_endpoint;
        static std::string clientid;
        static std::string apikey;
        static std::string token;
        static std::string sessionid;
        static signed int max_number_attempts;
        static bool connection_is_setup;
        static bool offline_mode;

        static void configure();
        static std::string get_token();
        static void send_event_attempt(std::string event);
        static Poco::Net::HTTPResponse::HTTPStatus post(std::string url, std::string payload, std::ostringstream& output_stream);
};
