/*
 * gameeventswrapper.cpp
 *
 *  Created on: 23 Nov 2015
 *      Author: maira
 */

#include "gameeventswrapper.h"


GameEventsWrapper::GameEventsWrapper(GameData passed_event_data, int passed_num_attempts) {
	event_data = passed_event_data;
	num_attempts = passed_num_attempts;
}

void GameEventsWrapper::run() {
	GameEvents::send_event(event_data, num_attempts);
}
