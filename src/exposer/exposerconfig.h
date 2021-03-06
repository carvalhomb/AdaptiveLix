/*
 * exposerconfig.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#pragma once

#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include <string>

class ExposerConfig {
public:
	ExposerConfig();
	~ExposerConfig();
	void initialize();
	void finalize();
private:
	void read_config_file();
	void update_globals();
	void get_sessionid();
	void get_sessionid_attempt();
	void handshake(std::string service, std::string resource = "version");
	void get_tokens();
	void get_gameevents_token(signed int number_of_attempts);
	void get_gameevents_token_attempt();
	void ping(std::string url);
	std::string extract_sessionid(std::string json_string);
	std::string extract_token(std::string json_string);
	void close_sessionid();
	void close_sessionid_attempt();
};
