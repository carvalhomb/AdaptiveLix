/*
 * pocowrapper.h
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include "gamedata.h"


class NetworkSaver  : public Poco::Runnable {

public:
	NetworkSaver(GameData passed_event_data, int passed_num_attempts = 3);
	//void save();
	virtual void run();
private:
//	void send_event(GameData event_data);
	void send_event(GameData event_data, signed int number_of_attempts);
	void send_event_attempt(std::string formatted_event);
	GameData event_data;
	int num_attempts;
};

