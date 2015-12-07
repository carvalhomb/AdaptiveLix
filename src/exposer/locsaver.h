/*
 * lsaver.h
 *
 *  Created on: 24 Nov 2015
 *      Author: mbrandaoca
 */

#include <string>
#include <Poco/RWLock.h>

#include "gamedata.h"

class LocalSaver {

	public:
		LocalSaver(GameData passed_event_data, Poco::RWLock* lock);
		void run();

	private:
		GameData event_data;
		void save_locally(std::string data_in_csv);
		static bool file_exists(std::string filename);
		Poco::RWLock* _lock;
};

