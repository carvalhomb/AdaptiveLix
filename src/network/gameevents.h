/*
 * Gameevents manage the formatting and sending of the
 * important game events to the external REST service
 * listening.
 */

#pragma once

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>
#include "gamedata.h"


class GameEvents
{
    public:
        static void send_event(GameData data);
        static void send_event(GameData data, signed int number_of_attempts);
        static void get_sessionid();

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
		static std::string format_event_data(GameData event_data);
		static std::string format_event_data_csv(GameData data);
		static bool file_exists(std::string filename);
        static void send_event_attempt(std::string event);
        static int post(std::string url, std::string payload, std::ostringstream& output_stream);
        static int post(std::string url, std::string payload, std::ostringstream& output_stream, std::string token);
        static void log_event_locally(GameData event_data);
        static std::string extract_sessionid(std::string);
        static std::string extract_token(std::string);
};
