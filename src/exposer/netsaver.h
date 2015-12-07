/*
 * pocowrapper.h
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */


#include "gamedata.h"


class NetworkSaver {

public:
	NetworkSaver(GameData event_data, std::string sessionid, int passed_num_attempts = 3);
	void run();
private:
	void send_event(GameData event_data, signed int number_of_attempts);
	void send_event_attempt(std::string formatted_event);
	GameData event_data;
	int num_attempts;
	std::string sessionid;
};

