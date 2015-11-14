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
			std::string timestamp;
			//time_t timestamp;
			int lix_required;
			int lix_saved;
			int skills_used;
			int seconds_required;

			Data();
			//virtual ~Data();
			void load_event_data(Replay::Data data, std::string level);
			void load_result_data(Result result, Level level);
			void prepare_event_data(std::string action_word, signed long update, std::string level);
		};

//		struct User {
//			std::string username;
//			std::string password;
//
//			User(std::string username, std::string password);
//			//virtual ~User();
//		};


        //GameEvents();
        //virtual ~GameEvents();


        static void send_event(GameEvents::Data data);
        static void send_event(GameEvents::Data data, signed int number_of_attempts);
        static void get_sessionid();



    protected:
    private:
        static std::string service_endpoint;
        static std::string clientid;
        static std::string apikey;
        static std::string token;
        static std::string sessionid;
        static std::string up_service_endpoint;
        static std::string username;
        static std::string password;
        static signed int max_number_attempts;
        static bool connection_is_setup;
        static bool record_local_file;
        static bool offline_mode;


        static void configure();
        static std::string get_token();
		static std::string format_event_data(GameEvents::Data event_data);
		static std::string format_event_data_csv(GameEvents::Data data);
		static bool file_exists(std::string filename);
        static void send_event_attempt(std::string event);
        static Poco::Net::HTTPResponse::HTTPStatus post(std::string url, std::string payload, std::ostringstream& output_stream);
        static void log_event_locally(GameEvents::Data event_data);
};
