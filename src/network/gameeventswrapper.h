/*
 * pocowrapper.h
 *
 *  Created on: 20 Nov 2015
 *      Author: mbrandaoca
 */

#include <Poco/Runnable.h>
#include "../network/gameevents.h"
#include "../network/gamedata.h"


class GameEventsWrapper : public Poco::Runnable {
//class GameEventsWrapper {

	public:
		GameEventsWrapper(GameData passed_event_data, int passed_num_attempts = 3);
		virtual void run();
		void run2();
		GameData event_data;
		int num_attempts;
};

