/*
 * notifcenter.cpp
 *
 *  Created on: 28 Nov 2015
 *      Author: maira
 */
#include "../other/myalleg.h" //needs to come first to include the windows.h header in the beginning

#include "gamedata.h"
#include "notification.h"


void GameEventNotification::load_data(GameData _data) {
	data = _data;
}

void GameEventNotification::load_sessionid(std::string _sessionid) {
	sessionid = _sessionid;
}




